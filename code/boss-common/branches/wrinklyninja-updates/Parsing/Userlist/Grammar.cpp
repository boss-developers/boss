/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Code file for userlist grammar definitions.

//#include "Grammar.h"



////////////////////////////
//Userlist Grammar.
////////////////////////////




/*
std::string context(error_pos, std::min(error_pos + 50, end));
		boost::trim_left(context);
		boost::replace_all(context, "\n", "<EOL>");

		std::cerr << "Syntax error while trying to parse Userlist.txt: '" << what << "' near this input: '" << context << "'." << std::endl;
	};

	void Rules::ParsingFailed(
			string::const_iterator	const& begin, 
			string::const_iterator	const& end, 
			string::const_iterator	const& error_pos, 
			string::difference_type lineNo)
	{
		string context(error_pos, std::min(error_pos + 50, end));
		boost::trim_left(context);
		boost::replace_all(context, "\n", "<EOL>");

		std::cerr << "Userlist.txt parsing error at line#: " << lineNo << " while reading near this input: '" << context << "'." << std::endl;
namespace { // Using an anonymous so local private declarations are only usable from inside this file.
		using namespace std;
		using boost::algorithm::to_lower_copy;
		using boost::algorithm::to_lower;
		using boost::algorithm::to_upper_copy;
		using boost::algorithm::trim_copy;
		using boost::format;

		// Error messages for rule validation
		static boost::format ESortLineInForRule("It includes a sort line in a rule with a FOR rule keyword.");
		static boost::format ERuleHasUndefinedObject("The line with keyword '%1%' has an undefined object.");
		static boost::format EPluginNotInstalled("'%1%' is not installed.");
		static boost::format EAddingModGroup("It tries to add a group.");
		static boost::format ESortingGroupEsms("It tries to sort the group \"ESMs\".");
		static boost::format ESortingMasterEsm("It tries to sort the master .ESM file.");
		static boost::format EReferencingModAndGroup("It references a mod and a group.");
		static boost::format ESortingGroupBeforeEsms("It tries to sort a group before the group \"ESMs\".");
		static boost::format ESortingModBeforeGameMaster("It tries to sort a mod before the master .ESM file.");
		static boost::format EInsertingToTopOfEsms("It tries to insert a mod into the top of the group \"ESMs\", before the master .ESM file.");
		static boost::format EInsertingGroupToGroupOrModToMod("It tries to insert a group or insert a mod into another mod.");
		static boost::format EAttachingMessageToGroup("It tries to attach a message to a group.");
		static boost::format EUnrecognisedKeyword("The line \"%1%: %2%\" does not contain a recognised keyword. If this line is the start of a rule, that rule will also be skipped.");
		static boost::format EAppearsBeforeFirstRule("The line \"%1%: %2%\" appears before the first recognised rule line. Line skipped.");
		static boost::format EUnrecognizedKeywordBeforeFirstRule("The line \"%1%: %2%\" does not contain a recognised keyword, and appears before the first recognised rule line. Line skipped.");

		static boost::format MessageParagraphFormat(
			"<p style='margin-left:40px; text-indent:-40px;'\n"
			 "	The rule beginning \" %1%: %2%\" has been skipped as it has the following problem(s):\n"
			 "	<br/>\n"
			 "	%3%\n"
			 "	<br/>\n"
			 "</p>\n"
			);

		static boost::format MessageSpanFormat(
			"	<span class='%1%'>\n"
			 "		%2%\n"
			 "	</span>\n"
			);

		// Used to throw as exception when signaling a rule parsing error, in order to make the code a bit more compact.
		struct failure
		{
			failure(bool skipped, string const& rule, string const& object, boost::format const& message) 
				: skipped(skipped), object(object), rule(rule), message(message)
			{}

			string object;
			string rule;
			format message;
			bool skipped;
		};
	}
	*/