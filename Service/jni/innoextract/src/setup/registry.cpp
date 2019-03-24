/*
 * Copyright (C) 2011-2013 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "setup/registry.hpp"

#include <boost/cstdint.hpp>

#include "setup/version.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

// 16-bit
STORED_ENUM_MAP(stored_registry_entry_type_0, registry_entry::None,
	registry_entry::None,
	registry_entry::String,
);

STORED_ENUM_MAP(stored_registry_entry_type_1, registry_entry::None,
	registry_entry::None,
	registry_entry::String,
	registry_entry::ExpandString,
	registry_entry::DWord,
	registry_entry::Binary,
	registry_entry::MultiString,
);

// starting with version 5.2.5
STORED_ENUM_MAP(stored_registry_entry_type_2, registry_entry::None,
	registry_entry::None,
	registry_entry::String,
	registry_entry::ExpandString,
	registry_entry::DWord,
	registry_entry::Binary,
	registry_entry::MultiString,
	registry_entry::QWord,
);

} // anonymous namespace

void registry_entry::load(std::istream & is, const version & version) {
	
	if(version < INNO_VERSION(1, 3, 21)) {
		(void)util::load<boost::uint32_t>(is); // uncompressed size of the entry
	}
	
	is >> util::encoded_string(key, version.codepage());
	if(version.bits != 16) {
		is >> util::encoded_string(name, version.codepage());
	} else {
		name.clear();
	}
	is >> util::encoded_string(value, version.codepage());
	
	load_condition_data(is, version);
	
	if(version >= INNO_VERSION(4, 0, 11) && version < INNO_VERSION(4, 1, 0)) {
		is >> util::encoded_string(permissions, version.codepage());
	} else {
		permissions.clear();
	}
	
	load_version_data(is, version);
	
	if(version.bits != 16) {
		hive = hive_name(util::load<boost::uint32_t>(is) & ~0x80000000);
	} else {
		hive = Unset;
	}
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		permission = util::load<boost::int16_t>(is);
	} else {
		permission = -1;
	}
	
	if(version >= INNO_VERSION(5, 2, 5)) {
		type = stored_enum<stored_registry_entry_type_2>(is).get();
	} else if(version.bits != 16) {
		type = stored_enum<stored_registry_entry_type_1>(is).get();
	} else {
		type = stored_enum<stored_registry_entry_type_0>(is).get();
	}
	
	stored_flag_reader<flags> flagreader(is, version.bits);
	
	if(version.bits != 16) {
		flagreader.add(CreateValueIfDoesntExist);
		flagreader.add(UninsDeleteValue);
	}
	flagreader.add(UninsClearValue);
	flagreader.add(UninsDeleteEntireKey);
	flagreader.add(UninsDeleteEntireKeyIfEmpty);
	flagreader.add(PreserveStringType);
	if(version >= INNO_VERSION(1, 3, 21)) {
		flagreader.add(DeleteKey);
		flagreader.add(DeleteValue);
		flagreader.add(NoError);
		flagreader.add(DontCreateKey);
	}
	if(version >= INNO_VERSION(5, 1, 0)) {
		flagreader.add(Bits32);
		flagreader.add(Bits64);
	}
	
	options = flagreader;
}

} // namespace setup

NAMES(setup::registry_entry::flags, "Registry Option",
	"create value if doesn't exist",
	"uninstall delete value",
	"uninstall clear value",
	"uninstall delete key",
	"uninstall delete key if empty",
	"preserve string type",
	"delete key",
	"delete value",
	"no error",
	"don't create key",
	"32 bit",
	"64 bit",
)

NAMES(setup::registry_entry::hive_name, "Registry Hive",
	"HKCR",
	"HKCU",
	"HKLM",
	"HKU",
	"HKPD",
	"HKCC",
	"HKDD",
	"Unset",
)

NAMES(setup::registry_entry::value_type, "Registry Entry Type",
	"none",
	"string",
	"expand string",
	"dword",
	"binary",
	"multi string",
	"qword",
)
