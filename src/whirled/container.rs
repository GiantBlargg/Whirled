use std::collections::HashMap;

pub trait ContainerGAT {
	type Handle: Copy;
	type Container<T>: Container<Self::Handle, T>;
}

pub struct CounterMapGAT {}
impl ContainerGAT for CounterMapGAT {
	type Handle = usize;
	type Container<T> = CounterMap<T>;
}

pub trait Container<K, V> {
	fn new() -> Self;
	fn reserve(&mut self, additional: usize);
	fn insert(&mut self, t: V) -> K;
	fn get(&self, k: &K) -> Option<&V>;
	type Drain: Iterator<Item = V>;
	fn drain(&mut self) -> Self::Drain;
}

pub struct CounterMap<V> {
	hash_map: HashMap<usize, V>,
	counter: usize,
}

impl<V> CounterMap<V> {}

impl<V> Container<usize, V> for CounterMap<V> {
	fn new() -> Self {
		Self {
			hash_map: HashMap::new(),
			counter: 0,
		}
	}
	fn reserve(&mut self, additional: usize) {
		self.hash_map.reserve(additional);
	}
	fn insert(&mut self, t: V) -> usize {
		let key = self.counter;
		self.counter += 1;
		assert!(self.hash_map.insert(key, t).is_none());
		key
	}
	fn get(&self, k: &usize) -> Option<&V> {
		self.hash_map.get(k)
	}

	type Drain = std::vec::IntoIter<V>;
	fn drain(&mut self) -> Self::Drain {
		self.hash_map
			.drain()
			.map(|(_, v)| v)
			.collect::<Vec<V>>()
			.into_iter()
	}
}
