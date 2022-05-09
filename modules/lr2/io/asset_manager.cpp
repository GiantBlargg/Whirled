#include "asset_manager.hpp"

#include <optional>

void AssetManager::_thread_func() {
	while (true) {
		wake_semaphore.acquire();
		if (shut_down)
			return;
		_find_work();
		std::this_thread::yield();
	}
}

// Caller should acquire wake_semaphore before calling this function
void AssetManager::_find_work() {
	// Find some work to do
	asset_cache_mutex.lock_shared(); // The asset_cache shouldn't move while we're reading it.
	for (auto& a : asset_cache) {
		if (a.second.state != AssetCache::State::QUEUED)
			continue; // Only work on queued assets

		// Found some queued work
		if (!a.second.work_semaphore.try_acquire())
			continue; // Someone stole the work, keep looking

		// Acquired the work_semaphore, now we can do the work
		AssetKey key = a.first;
		_do_work(key, true);
		return;
	}
	asset_cache_mutex.unlock_shared();
	ERR_FAIL_MSG("Thread tried to do work but there was no work!");
}

void AssetManager::_do_work(const AssetKey& key, bool asset_cache_locked) {

	// Let others know that we're working on it
	if (!asset_cache_locked) {
		asset_cache_mutex.lock_shared();
	}
	asset_cache[key].state = AssetCache::State::WORKING;
	asset_cache_mutex.unlock_shared();

	loader_mutex.lock_shared();
	Ref<AssetLoader> loader;
	for (auto& l : loaders) {
		if (l->can_handle(key, custom_fs)) {
			loader = l;
			break;
		}
	}
	loader_mutex.unlock_shared();

	if (loader.is_null()) {
		asset_cache_mutex.lock_shared();
		asset_cache[key].mutex.lock();
		asset_cache[key].state = AssetCache::State::FAILED;
		asset_cache[key].mutex.unlock();
		asset_cache_mutex.unlock_shared();
		print_error("Could not find loader for {" + key.path + ", " + key.type + "}");
		return;
	}

	REF asset;
	AssetKey remap_key = loader->remap_key(key, custom_fs);
	if (key == remap_key) {
		// No remap, let's load it!
		asset = loader->load(key, custom_fs, *this);
	} else {
		// Remap needed
		asset = block_get(remap_key);
	}

	asset_cache_mutex.lock_shared();
	asset_cache[key].mutex.lock();
	asset_cache[key].asset = asset;
	asset_cache[key].state = AssetCache::State::COMPLETE;
	asset_cache[key].mutex.unlock();
	asset_cache_mutex.unlock_shared();
}

void AssetManager::vector_queue(const Vector<AssetKey>& p_keys) {
	Vector<AssetKey> keys(p_keys);
	_canon_paths(keys);
	Vector<AssetKey> new_keys;
	asset_cache_mutex.lock();
	for (auto& k : keys) {
		if (!asset_cache.count(k))
			if (!new_keys.has(k))
				new_keys.push_back(k);
	}
	asset_cache_mutex.unlock();

	_vector_queue(new_keys);
}

void AssetManager::_vector_queue(const Vector<AssetKey>& p_keys) {
	if (p_keys.is_empty())
		return;

	int wake = 0;
	asset_cache_mutex.lock();
	for (auto& k : p_keys) {
		if (!asset_cache.count(k)) {
			asset_cache[k].work_semaphore.release();
			asset_cache[k].state = AssetCache::State::QUEUED;
			wake++;
		}
	}
	asset_cache_mutex.unlock();
	wake_semaphore.release(wake);
}

void AssetManager::_canon_paths(Vector<AssetKey>& keys) {
	for (auto& k : keys) {
		k.path = custom_fs.canon_path(k.path);
	}
}

Vector<REF> AssetManager::vector_try_get(const Vector<AssetKey>& p_keys) {
	if (thread_pool.size() == 0) {
		// There's no threads so waiting would be pointless
		return vector_block_get(p_keys);
	}

	Vector<AssetKey> keys(p_keys);
	_canon_paths(keys);

	Vector<AssetKey> new_keys;

	Vector<REF> ret;
	ret.resize(keys.size());

	asset_cache_mutex.lock_shared();
	for (int i = 0; i < keys.size(); i++) {
		const AssetKey& k = keys[i];
		if (asset_cache.count(k)) {
			if (asset_cache[k].state == AssetCache::State::COMPLETE) {
				asset_cache[k].mutex.lock();
				ret.set(i, asset_cache[k].asset);
				asset_cache[k].mutex.unlock();
			}
		} else {
			if (!new_keys.has(k))
				new_keys.push_back(k);
		}
	}
	asset_cache_mutex.unlock_shared();

	_vector_queue(new_keys);

	return ret;
}

Vector<REF> AssetManager::vector_block_get(const Vector<AssetKey>& p_keys) {
	Vector<AssetKey> keys(p_keys);
	_canon_paths(keys);

	Vector<AssetKey> new_keys; // Keys that need to be queued
	Vector<AssetKey> queued;   // Keys that this thread could work on while waiting
	Vector<int> not_ready;     // Keys not loaded

	Vector<REF> ret;
	ret.resize(keys.size());

	asset_cache_mutex.lock_shared();
	for (int i = 0; i < keys.size(); i++) {
		const AssetKey& k = keys[i];
		if (asset_cache.count(k)) {
			AssetCache::State state = asset_cache.at(k).state;
			if (state == AssetCache::State::COMPLETE) {
				asset_cache[k].mutex.lock();
				ret.set(i, asset_cache[k].asset);
				asset_cache[k].mutex.unlock();
			} else if (state == AssetCache::State::QUEUED) {
				if (!queued.has(k))
					queued.push_back(k);
				not_ready.push_back(i);
			} else if (state == AssetCache::State::WORKING) {
				not_ready.push_back(i);
			}
		} else {
			if (!queued.has(k)) {
				queued.push_back(k);
				new_keys.push_back(k);
			}
			not_ready.push_back(i);
		}
	}
	asset_cache_mutex.unlock_shared();

	if (not_ready.is_empty())
		// We got everything already!
		return ret;

	if (!new_keys.is_empty()) {
		// Some keys haven't been queued yet

		// Don't queue one key that we can work on locally instead of fighting the pool for work
		AssetKey work_key;
		bool should_do_work = false;
		int wake = 0;
		asset_cache_mutex.lock();
		for (auto k : new_keys) {
			// Key could have been queued by another thread
			if (!asset_cache.count(k)) {
				if (!should_do_work) {
					asset_cache[k].state = AssetCache::State::INIT;
					work_key = k;
					should_do_work = true;
				} else {
					asset_cache[k].work_semaphore.release();
					asset_cache[k].state = AssetCache::State::QUEUED;
					wake++;
				}
			}
		}
		asset_cache_mutex.unlock();
		wake_semaphore.release(wake);

		// _vector_queue(new_keys); // Do queueing here to avoid extra locking

		// Because this key wasn't queued we don't need to steal it from the pool
		if (should_do_work) {
			_do_work(work_key);
		}
	}

	if (!queued.is_empty()) {
		// Maybe we can steal some work from the pool instead of waiting
		auto queued_iter = queued.begin();
		while (wake_semaphore.try_acquire()) {
			// We've stolen a work slot from the pool

			// Look for an item we need that isn't started yet
			std::optional<AssetKey> work;
			asset_cache_mutex.lock_shared();
			while (queued_iter != queued.end()) {
				if (asset_cache[*queued_iter].work_semaphore.try_acquire()) {
					// Got the work. Time to do it!
					work = *queued_iter;
					break;
				}

				// Another thread took this work, keep looking
				++queued_iter;
			}
			if (!work.has_value()) {
				asset_cache_mutex.unlock_shared();
				// No more relevant work to do
				wake_semaphore.release(); // We took the work slot but didn't use it
				break;
			}

			_do_work(work.value(), true);
		}
	}

	// All done relevant work

	while (true) {
		Vector<int> new_not_ready;

		asset_cache_mutex.lock_shared();
		for (auto i : not_ready) {
			const AssetKey& k = keys[i];
			AssetCache::State state = asset_cache.at(k).state;
			if (state == AssetCache::State::COMPLETE) {
				asset_cache[k].mutex.lock();
				ret.set(i, asset_cache[k].asset);
				asset_cache[k].mutex.unlock();
			} else if (state == AssetCache::State::QUEUED) {
				new_not_ready.push_back(i);
			} else if (state == AssetCache::State::WORKING) {
				new_not_ready.push_back(i);
			}
		}
		asset_cache_mutex.unlock_shared();

		if (new_not_ready.is_empty()) {
			// Should have everything now
			return ret;
		}

		not_ready = new_not_ready;

		std::this_thread::yield();
	}
}

AssetManager::AssetManager(const CustomFS& p_custom_fs) : custom_fs(p_custom_fs) {
	uint num_threads = MAX(std::thread::hardware_concurrency() - 1, 0);
	thread_pool.reserve(num_threads);

	for (int i = 0; i < num_threads; i++) {
		thread_pool.emplace_back(&AssetManager::_thread_func, this);
	}
}
AssetManager::~AssetManager() {
	shut_down = true;
	wake_semaphore.release(thread_pool.size());
	for (auto& t : thread_pool) {
		t.join();
	}
}
