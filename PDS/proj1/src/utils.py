"""
Author: Adam Zvara (xzvara01@stud.fit.vutbr.cz)
Date: 3/2024
"""

class Log:
    """ Simple logging class for debugging purposes """
    def __init__(self, component: str, do_print: bool = True):
        self.component = component
        self.do_print = do_print
        
    def log(self, message: str):
        if self.do_print:
            print(f'{self.component}: {message}')