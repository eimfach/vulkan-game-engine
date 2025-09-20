#pragma once


//std
#include <vector>
#include <mutex>
#include <queue>

namespace nEngine {
	template <class T>
	class BufferedVector {
	public:
		BufferedVector() {}
		~BufferedVector() {}
		BufferedVector(BufferedVector&& other) = delete;

		void reserve(size_t size) {
			data.reserve(size);
		}
		void push(T element) {
			std::lock_guard<std::mutex> lock{ mutex };

			buffer.push(element);
		}
		const std::vector<T>& get() {
			return data;
		}
		std::vector<T>& get_writable() {
			return data;
		}
		bool sync_element() {
			std::lock_guard<std::mutex> lock{ mutex };

			if (!buffer.empty()) {
				T front = buffer.front();
				data.push_back(front);
				buffer.pop();
				return true;
			}

			return false;
		}
	private:
		std::mutex mutex{};
		std::vector<T> data{};
		std::queue<T> buffer{};
	};
}
