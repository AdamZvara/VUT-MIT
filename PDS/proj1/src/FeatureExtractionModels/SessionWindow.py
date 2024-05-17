"""
Session window extraction

Author: Adam Zvara (xzvara01@stud.fit.vutbr.cz)
Date: 3/2024
"""

import pandas as pd
import numpy as np
import re
from collections import OrderedDict

from ..utils import Log

class SessionBasedExtraction(Log):
    """ Vectorize the log sequences into sessions based windows based on session id """
    
    def __init__(self, logging = True):
        super().__init__(self.__class__.__name__, logging)
        
    def transform(self, x_data, event_col, session_reg, session_col, y_data = None):
        """ Transform raw logs into session windows
        
        ### Args:
            x_data (DataFrame): raw log data
            session_reg (str): regular expression to extract session id from log message
            session_col (str): name of the column containing the log message
            y_data (Array): labels for each log message
            
        ### Returns:
            X_df (DataFrame): feature matrix with column names as event ids, each row represents a session
            Y (Array): label matrix with column "Label" column containing labels for each session 
            
        ### References
            Original code can be found at: https://github.com/logpai/loglizer/blob/master/loglizer/dataloader.py
                and https://github.com/logpai/loglizer/blob/master/loglizer/preprocessing.py
            Authors: LogPAI Team
        """        
        # Collect all events and labels for each session id
        self.log(f"Collecting events")
        data_dict = OrderedDict()
        labels_dict = {}
        for idx, row in x_data.iterrows():
            blkId_list = re.findall(session_reg, row[session_col])
            blkId_set = set(blkId_list)
            # Add the session id to the dictionary
            for blk_Id in blkId_set:
                if not blk_Id in data_dict:
                    data_dict[blk_Id] = []
                data_dict[blk_Id].append(row[event_col])
                
                # Check if the session id has the same label in all logs
                if y_data is not None:
                    if not blk_Id in labels_dict:
                        labels_dict[blk_Id] = y_data[idx]
                    else:
                        assert labels_dict[blk_Id] == y_data[idx], "Session id must have the same label in all logs"
         
        # Create a dataframe from the list of tuples
        X_df = pd.DataFrame(list(data_dict.items()), columns=["SessionId", event_col])
        Y = np.array(list(labels_dict.values()))
        
        if y_data is not None:
            assert X_df.shape[0] == len(Y), "Number of sessions must match the number of session ids"
        else:
            Y = None
        
        return X_df, Y
    
