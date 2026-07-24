#pragma once
#include "ExtraTypes.h"
#include <vector>

// Scans "<mod>\GameDLC\*.list" for user-provided definitions that expand the
// hard-coded official DLC list (see VehicleHashes.h / buildDLClist()).
// Files are named "<key>_<Name>.list", where <key> is used purely for
// ordering entries relative to each other (e.g. a date like "2024-06"), and
// <Name> is the DLC display name. Any file whose Name case-insensitively
// matches a DLC already present in existingDlcs is skipped, so an official
// script update transparently supersedes a user-defined entry.
std::vector<DLCDefinition> BuildGameDLCList(const std::vector<DLCDefinition>& existingDlcs);
