"""
API to load datasets from structured log files

Author: Adam Zvara (xzvara01@stud.fit.vutbr.cz)
Date: 3/2024
"""

import pandas as pd
from .utils import Log

class DataLoader(Log):
    def __init__(self, logging = True):
        super().__init__(self.__class__.__name__, logging)
        
    def load_csv(self, data_path, label_path = None, labels_col = "Label"):
        """ Load the structured log data (and labels) from CSV files 
        
        ### Args:
            data_path (str): path to the structured log file
            label_path (str): path to the file with labels (default = None)
            labels_col (str): name of the column with labels in the label file (default = "Label")
            
        ### Notes:
            The label file must contain only binary values (e.g Normal = 0, Anomaly = 1)
        
        ### Returns:
            X(DataFrame), Y(Numpy array): training and validation sets with optional labels
        """
        self.log(10 * "-" + " Loading data " + 10 * "-")
        
        # Load the structured log data from the file
        self.log(f"Loading data from \"{data_path}\",")
        assert data_path.endswith(".csv"), "Only CSV files are supported"
        X = pd.read_csv(data_path, engine='c', na_filter=False, memory_map=True, skipinitialspace=True)
        Y = None
        
        # If label file is provided, load the labels and merge them with the structured log data
        if label_path is not None:
            self.log(f"Loading labels from \"{label_path}\"") 
            assert label_path.endswith(".csv"), "Only CSV files are supported"
            Y = pd.read_csv(label_path, engine='c', na_filter=False, memory_map=True, skipinitialspace=True)
            # Convert labels to binary format and return them as an array
            Y = self._labels_to_binary(Y, labels_col)
            assert X.shape[0] == len(Y), "Number of samples in the data and label files must be the same" 
            
        return X, Y
    
    def _labels_to_binary(self, Y_df, labels_col):
        """ Convert labels to binary format (0, 1) 
        
        ### Args:
            Y_df (DataFrame): DataFrame with labels
            labels_col (str): name of the column with labels in the DataFrame
        """
        labels = set(Y_df[labels_col])
        if labels == {0, 1}: # Already in binary format
            return Y_df[labels_col]
        elif labels == {"Normal", "Anomaly"}: 
            Y = Y_df[labels_col].apply(lambda x: 0 if x == "Normal" else 1)
            return Y
        else:
            raise ValueError("Only binary labels are supported (0, 1) or (Normal, Anomaly)")