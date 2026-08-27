#include "encodings_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension/extension_loader.hpp"

#include "generated_encoded_function.hpp"
#include "generated/registration.hpp"

namespace duckdb {

static void LoadInternal(ExtensionLoader &loader) {
	// Register a scalar function
	auto &config = loader.GetDatabaseInstance().config;
	duckdb_encodings::RegistrationEncodedFunctions::RegisterFunctions(config);
}

void EncodingsExtension::Load(ExtensionLoader &loader) {
	LoadInternal(loader);
}
std::string EncodingsExtension::Name() {
	return "encodings";
}

std::string EncodingsExtension::Version() const {
#ifdef EXT_VERSION_ENCODINGS
	return EXT_VERSION_ENCODINGS;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_CPP_EXTENSION_ENTRY(encodings, loader) {
	duckdb::LoadInternal(loader);
}
}
