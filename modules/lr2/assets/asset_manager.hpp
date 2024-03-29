#pragma once

#include <semaphore>
#include <thread>
#include <unordered_map>
#include <vector>

#include "core/object/ref_counted.h"
#include "core/templates/pair.h"

#include "lr2/io/custom_fs.hpp"

struct AssetKey {
	String path;
	String type;
};
inline bool operator==(const AssetKey& pair, const AssetKey& other) {
	return (pair.path == other.path) && (pair.type == other.type);
}
class AssetManager;

class AssetLoader : public RefCounted {
	GDCLASS(AssetLoader, RefCounted);

  public:
	virtual bool can_handle(const AssetKey&, const CustomFS&) const = 0;
	virtual AssetKey remap_key(const AssetKey&, const CustomFS&) const = 0;
	virtual Ref<RefCounted> load(const AssetKey&, const CustomFS&, AssetManager&, Error* r_error = nullptr) const = 0;
	virtual ~AssetLoader() {}
};

class AssetManager {

  public:
	const CustomFS custom_fs;

  private:
	std::vector<std::thread> thread_pool;
	std::counting_semaphore<> wake_semaphore{0}; // Released when there is work to do
	volatile bool shut_down = false;

	void _thread_func();
	void _find_work(); // Caller should acquire wake_semaphore before calling this function
	void _do_work(const AssetKey& key, bool asset_cache_locked = false);

	struct AssetCache {
		enum class State { INIT, QUEUED, WORKING, COMPLETE, FAILED };
		volatile State state = State::INIT;
		Ref<RefCounted> asset;
		std::mutex mutex;                        // Lock when modifying the above fields of this structure
		std::binary_semaphore work_semaphore{0}; // Released when queued for work, acquire before doing the work.
	};
	struct Hasher {
		size_t operator()(const AssetKey& key) const { return ((key.path) + (key.type)).hash64(); }
	};
	std::unordered_map<AssetKey, AssetCache, Hasher> asset_cache;
	std::shared_mutex asset_cache_mutex;

  public:
	void vector_queue(const Vector<AssetKey>&);
	Vector<Ref<RefCounted>> vector_try_get(const Vector<AssetKey>&);
	Vector<Ref<RefCounted>> vector_block_get(const Vector<AssetKey>&);

  private:
	void _vector_queue(const Vector<AssetKey>&);
	void _canon_paths(Vector<AssetKey>&);

	template <class T> Vector<AssetKey> _type_keys(const Vector<String>& paths) {
		Vector<AssetKey> ret;
		ret.resize(paths.size());
		String type = T::get_class_static();
		for (int i = 0; i < paths.size(); i++) {
			ret.set(i, AssetKey{paths[i], type});
		}
		return ret;
	}

  public:
	template <class T> void queue(const String& p_path) { queue({p_path, T::get_class_static()}); }
	void queue(const AssetKey& key) { vector_queue({key}); }
	template <class T> void vector_queue(const Vector<String>& paths) { vector_queue(_type_keys<T>(paths)); }

	template <class T> Ref<T> try_get(const String& p_path) { return try_get({p_path, T::get_class_static()}); }
	Ref<RefCounted> try_get(const AssetKey& key) { return vector_try_get({key})[0]; }

	template <class T> Ref<T> block_get(const String& p_path) { return block_get({p_path, T::get_class_static()}); }
	Ref<RefCounted> block_get(const AssetKey& key) { return vector_block_get({key})[0]; }

  private:
	Vector<Ref<AssetLoader>> loaders;
	std::shared_mutex loader_mutex;

  public:
	template <class T> void add_loader() { add_loader(Ref(memnew(T))); }
	void add_loader(Ref<AssetLoader> loader) {
		loader_mutex.lock();
		loaders.push_back(loader);
		loader_mutex.unlock();
	}

  public:
	AssetManager(const CustomFS&);
	~AssetManager();
};