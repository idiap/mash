#include <UnitTest++.h>
#include <mash-network/commands_serializer.h>

using namespace Mash;

SUITE(CommandsSerializerSuite)
{
    TEST(NoCommandsAtCreation)
    {
        CommandsSerializer serializer;

        CHECK_EQUAL(0, serializer.nbCommands());
    }


    TEST(FailToDeserializeAnUnknownFile)
    {
        CommandsSerializer serializer;

        CHECK(!serializer.deserialize("unknown.txt"));
    }


    TEST(DeserializationOfAnEmptyFile)
    {
        CommandsSerializer serializer;

        CHECK(serializer.deserialize(MASH_DATA_DIR "/unittests/empty.txt"));
        
        CHECK_EQUAL(0, serializer.nbCommands());
    }


    TEST(DeserializationOfOneCommand)
    {
        CommandsSerializer serializer;

        CHECK(serializer.deserialize(MASH_DATA_DIR "/unittests/one_command.txt"));
        
        CHECK_EQUAL(1, serializer.nbCommands());
        
        CommandsSerializer::tCommand command = serializer.getCommand(0);

        CHECK_EQUAL("COMMAND1", command.strCommand);
        CHECK_EQUAL(0, command.arguments.size());
    }


    TEST(DeserializationOfOneCommandWithArguments)
    {
        CommandsSerializer serializer;

        CHECK(serializer.deserialize(MASH_DATA_DIR "/unittests/one_command_with_arguments.txt"));
        
        CHECK_EQUAL(1, serializer.nbCommands());

        CommandsSerializer::tCommand command = serializer.getCommand(0);

        CHECK_EQUAL("COMMAND1", command.strCommand);
        CHECK_EQUAL(2, command.arguments.size());
        CHECK_EQUAL("ARG1", command.arguments.getString(0));
        CHECK_EQUAL("ARG2", command.arguments.getString(1));
    }


    TEST(InvalidCommandIndexReturnsAnEmptyCommand)
    {
        CommandsSerializer serializer;

        CHECK(serializer.deserialize(MASH_DATA_DIR "/unittests/one_command.txt"));
        
        CHECK_EQUAL(1, serializer.nbCommands());
        
        CommandsSerializer::tCommand command = serializer.getCommand(10);

        CHECK_EQUAL("", command.strCommand);
        CHECK_EQUAL(0, command.arguments.size());
    }


    TEST(DeserializationOfThreeCommands)
    {
        CommandsSerializer serializer;

        CHECK(serializer.deserialize(MASH_DATA_DIR "/unittests/three_commands.txt"));
        
        CHECK_EQUAL(3, serializer.nbCommands());
        
        CommandsSerializer::tCommand command = serializer.getCommand(0);

        CHECK_EQUAL("COMMAND1", command.strCommand);
        CHECK_EQUAL(0, command.arguments.size());

        command = serializer.getCommand(1);

        CHECK_EQUAL("COMMAND2", command.strCommand);
        CHECK_EQUAL(0, command.arguments.size());

        command = serializer.getCommand(2);

        CHECK_EQUAL("COMMAND3", command.strCommand);
        CHECK_EQUAL(0, command.arguments.size());
    }


    TEST(DeserializationOfOneCommandWithQuotedStringArgument)
    {
        CommandsSerializer serializer;

        CHECK(serializer.deserialize(MASH_DATA_DIR "/unittests/one_command_with_quoted_string_argument.txt"));
        
        CHECK_EQUAL(1, serializer.nbCommands());

        CommandsSerializer::tCommand command = serializer.getCommand(0);

        CHECK_EQUAL("COMMAND1", command.strCommand);
        CHECK_EQUAL(1, command.arguments.size());
        CHECK_EQUAL("This is a 'complicated' and\nmultiline string argument", command.arguments.getString(0));
    }


    TEST(DeserializationOfOneCommentedCommand)
    {
        CommandsSerializer serializer;

        CHECK(serializer.deserialize(MASH_DATA_DIR "/unittests/one_commented_command.txt"));
        
        CHECK_EQUAL(1, serializer.nbCommands());

        CommandsSerializer::tCommand command = serializer.getCommand(0);

        CHECK_EQUAL("COMMAND1", command.strCommand);
        CHECK_EQUAL(0, command.arguments.size());
    }


    TEST(DeserializationOfEmptyLines)
    {
        CommandsSerializer serializer;

        CHECK(serializer.deserialize(MASH_DATA_DIR "/unittests/two_commands_with_empty_line.txt"));
        
        CHECK_EQUAL(2, serializer.nbCommands());

        CommandsSerializer::tCommand command = serializer.getCommand(0);

        CHECK_EQUAL("COMMAND1", command.strCommand);
        CHECK_EQUAL(0, command.arguments.size());

        command = serializer.getCommand(1);

        CHECK_EQUAL("COMMAND2", command.strCommand);
        CHECK_EQUAL(0, command.arguments.size());
    }
}
