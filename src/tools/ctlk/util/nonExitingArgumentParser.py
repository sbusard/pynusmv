import argparse

class NonExitingArgumentParser(argparse.ArgumentParser):
    """An ArgumentParser that does not exit."""
    
    def exit(self, status=0, message=None):
        raise ArgumentParsingError(message)


class ArgumentParsingError(Exception):
    """An error occured while parsing arguments."""
    pass