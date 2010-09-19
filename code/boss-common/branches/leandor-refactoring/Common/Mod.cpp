#include "Mods.h"
#include <boost/algorithm/string.hpp>

namespace boss {

Mod::Mod(const fs::path& path) : path(path) 
{
	this->ghosted = (path.extension() == ".ghost");
	fs::path temp = path;
	if (ghosted) {
		temp = fs::change_extension(path, "");
	}
	this->name = Name(temp);
	this->id = ID(temp);
	this->lastWriteTime = fs::last_write_time(this->path);
}

const string& Mod::Name( const fs::path& path )
{
	return path.filename();
}

const string& Mod::ID( const fs::path& path )
{
	return Tidy(Name(path));
}

void Mod::LastWriteTime( const time_t& value )
{
	try { 
		fs::last_write_time(this->path, value);
	} catch(fs::filesystem_error e) {
		throw SortFailed("Error: Could not change the date of \"" + name + "\", check the Troubleshooting section of the ReadMe for more information and possible solutions.");
	}
}

bool Mod::IsMod( const fs::path& path )
{
	using namespace boost;

	const bool is_regular_file = fs::is_regular_file(path);

	if (!is_regular_file) 
		return false;
	
	const string& ext = to_lower_copy(path.extension());

	return  (ext == ".esp" || ext == ".esm" || ext == ".ghost");
}

}