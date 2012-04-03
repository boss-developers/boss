Use this program to check the validity of the regular expressions extracting the version string from mod's descriptions.

To use it, copy the VersionCheck.txt to the Release directory where the program is located and run the EXE, it will dump to the console the result of evaluating every mod on  the chack list (VersionCheck.txt), and if there are any failures, it will dump the mods that didn't pass the check.

The check is basically:

- Read a VersionCheck.txt entry,
- Get the Description from it
- Parse the string to get the version
- Check that version extracted to the one the VersionCheck.txt says it should have
- If different => fail the check

The VersionCheck.txt can be generated for your whole load order using the DumpHeaders project. Run it with your Oblivion\Data as working directory and it will create a new modinfo.txt file with the content of all the ESP/ESMs found on yout Data folder in the correct format for the RegexTesting. 

Before using that file, edit by hand the versions missed by the extraction process, then you can try to edit the regexes and test them using the RegextTesting.exe to try your changes.