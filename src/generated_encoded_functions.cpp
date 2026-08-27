#include "generated_encoded_function.hpp"

#include "duckdb/execution/operator/csv_scanner/encode/csv_encoder.hpp"

namespace duckdb {
namespace duckdb_encodings {

void GeneratedEncodedFunction::Decode(CSVEncoderBuffer &encoded_buffer, char *target_buffer,
                                      idx_t &target_buffer_current_position, const idx_t target_buffer_size,
                                      char *remaining_bytes_buffer, idx_t &remaining_bytes_size,
                                      EncodingFunction *encoding_function) {
	const auto encoded_buffer_ptr = encoded_buffer.Ptr();
	const int lookup_bytes = static_cast<int>(encoding_function->GetLookupBytes());
	while (encoded_buffer.cur_pos < encoded_buffer.actual_encoded_buffer_size) {
		// We need to use our map from the highest to lowest lookup bytes
		if (encoded_buffer.actual_encoded_buffer_size - encoded_buffer.cur_pos < lookup_bytes &&
		    !encoded_buffer.last_buffer) {
			// Not enough bytes to check.
			return;
		}
		int byte_group = lookup_bytes;
		if (encoded_buffer.actual_encoded_buffer_size - encoded_buffer.cur_pos < lookup_bytes) {
			byte_group = static_cast<int>(encoded_buffer.actual_encoded_buffer_size - encoded_buffer.cur_pos);
		}
		bool did_replacement = false;
		for (; byte_group > 0; --byte_group) {
			// We found a match, do the conversion
			const auto it = FindEntry(encoding_function->conversion_map, encoding_function->map_size,
			                          &encoded_buffer_ptr[encoded_buffer.cur_pos], byte_group);
			if (it != nullptr) {
				// We walk the byte group size from our buffer source
				did_replacement = true;
				encoded_buffer.cur_pos += byte_group;
				for (idx_t byte_pos = 0; byte_pos < it->value_len; ++byte_pos) {
					if (target_buffer_current_position == target_buffer_size) {
						// We are done, but we have to store one byte for the next chunk!
						remaining_bytes_size = it->value_len - byte_pos;
						for (idx_t remaining_pos = 0; remaining_pos < remaining_bytes_size; ++remaining_pos) {
							remaining_bytes_buffer[remaining_pos] =
							    static_cast<char>(it->value[byte_pos + remaining_pos]);
						}
						return;
					}
					target_buffer[target_buffer_current_position++] = it->value[byte_pos];
				}
				// We finished off this group
				break;
			}
		}
		if (!did_replacement) {
			// If we got here, it means replacements are not necessary
			target_buffer[target_buffer_current_position++] = encoded_buffer_ptr[encoded_buffer.cur_pos++];
		}
	}
}

bool GeneratedEncodedFunction::KeyLess(const char *a, size_t a_len, const char *b, size_t b_len) {
	const size_t min_len = std::min(a_len, b_len);
	const int cmp = std::memcmp(a, b, min_len);
	if (cmp != 0) {
		return cmp < 0;
	}
	return a_len < b_len;
}

struct MapEntryComparator {
	bool operator()(const map_entry_encoding &entry, const std::pair<const char *, size_t> &key) const {
		return GeneratedEncodedFunction::KeyLess(entry.key, entry.key_len, key.first, key.second);
	}
};

const map_entry_encoding *GeneratedEncodedFunction::FindEntry(const map_entry_encoding *map, size_t map_size,
                                                              const char *search_key, size_t search_len) {
	const auto it =
	    std::lower_bound(map, map + map_size, std::make_pair(search_key, search_len), MapEntryComparator {});
	if (it != map + map_size && it->key_len == search_len && std::memcmp(it->key, search_key, search_len) == 0) {
		return it;
	}
	return nullptr;
}

} // namespace duckdb_encodings

} // namespace duckdb
