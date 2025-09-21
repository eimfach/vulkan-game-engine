#pragma once


//std
#include <vector>
#include <mutex>
#include <queue>
#include <type_traits>

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

		void pushBuffer(T element) {
			std::lock_guard<std::mutex> lock{ mutex };

			buffer.push(element);
		}

		T& back() {
			return data.back();
		}
		
		size_t size() {
			return data.size();
		}

		const std::vector<T>& get() {
			return data;
		}

		std::vector<T>& getWritable() {
			return data;
		}

		bool syncElement() {
			std::lock_guard<std::mutex> lock{ mutex };

			if (!buffer.empty()) {
				data.push_back(std::move(buffer.front()));
				buffer.pop();
				return true;
			}

			return false;
		}

		void mergeElement(T& t) {
			std::lock_guard<std::mutex> lock{ mutex };

			if (!buffer.empty()) {
				T& back = data.back();
				back += t;
			}
		}

	private:
		std::mutex mutex{};
		std::vector<T> data{};
		std::queue<T> buffer{};

	static_assert(std::is_move_constructible_v<T>);
	static_assert(std::is_move_assignable_v<T>);
	};

	
}
