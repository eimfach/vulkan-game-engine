#pragma once

//std
#include <mutex>
#include <queue>
#include <type_traits>
#include <vector>
#include <stdexcept>

namespace nEngine {

	enum class SyncMode {
		create,
		replace,
		end
	};

	template <class VectorT, class BufferT = VectorT>
	class BufferedVector {
	public:
		SyncMode mode{ SyncMode::create };

		BufferedVector() {}
		~BufferedVector() {}
		BufferedVector(BufferedVector&& other) = delete;

		void reserve(size_t size);
		void pushBuffer(BufferT element);
		VectorT& back();
		size_t size();
		const std::vector<VectorT>& get();
		std::vector<VectorT>& getWriteable();
		bool syncElement();

	private:
		std::mutex mutex{};
		std::vector<VectorT> data{};
		std::queue<BufferT> buffer{};

		void _sync();
		void _merge();

		static_assert(std::is_move_constructible_v<BufferT>);
		static_assert(std::is_move_assignable_v<BufferT>);
	};

	template<class VectorT, class BufferT>
	void BufferedVector<VectorT, BufferT>::reserve(size_t size) {
		data.reserve(size);
	}

	template<class VectorT, class BufferT>
	void BufferedVector<VectorT, BufferT>::pushBuffer(BufferT element) {
		std::lock_guard<std::mutex> lock{ mutex };

		buffer.emplace(element);
	}

	template<class VectorT, class BufferT>
	VectorT& BufferedVector<VectorT, BufferT>::back() {
		return data.back();
	}

	template<class VectorT, class BufferT>
	size_t BufferedVector<VectorT, BufferT>::size() {
		return data.size();
	}

	template<class VectorT, class BufferT>
	const std::vector<VectorT>& BufferedVector<VectorT, BufferT>::get() {
		return data;
	}

	template<class VectorT, class BufferT>
	std::vector<VectorT>& BufferedVector<VectorT, BufferT>::getWriteable() {
		return data;
	}

	template<class VectorT, class BufferT>
	void BufferedVector<VectorT, BufferT>::_sync() {
		static size_t replacer_index = 0;

		switch (mode) {
		case SyncMode::create:
			data.push_back(std::move(buffer.front()));
			break;
		case SyncMode::replace:
			data.at(replacer_index) = buffer.front();

			if (replacer_index == data.size() - 1) {
				replacer_index = 0;
				mode = SyncMode::end;
			}
			else {
				replacer_index += 1;
			}
			break;

		case SyncMode::end:
			throw std::runtime_error("Tried to sync BufferedVector in an ended state");
			break;
		}
		
		buffer.pop();
	}

	template<class VectorT, class BufferT>
	void BufferedVector<VectorT, BufferT>::_merge() {
		VectorT& back = data.back();
		back += buffer.front();
		buffer.pop();
	}

	template<class VectorT, class BufferT>
	bool BufferedVector<VectorT, BufferT>::syncElement() {
		std::lock_guard<std::mutex> lock{ mutex };

		if (!buffer.empty()) {
			_sync();
			return true;
		}

		return false;
	}


}

