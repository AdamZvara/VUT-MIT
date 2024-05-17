"""
Time windows (fixed or sliding) extraction

Author: Adam Zvara (xzvara01@stud.fit.vutbr.cz)
Date: 3/2024
"""

import pandas as pd
import numpy as np
from collections import OrderedDict

from ..utils import Log

class WindowParams:
    """ Parameters for windowing 
    
    ### Args:
        window_size (int): size of the window in minutes
        window_step (int): step of the window in seconds
        time_col (str): name of the column containing the time
        time_fmt (str): format of the time column
        date_col (list[str]): name of the column containing the date
        date_fmt (str): format of the date column
    """
    def __init__(self, window_size, window_step, time_col, time_fmt, date_col = None, date_fmt = None):
        self.window_size = window_size
        self.window_step = window_step
        self.time_col = time_col
        self.time_fmt = time_fmt
        self.date_col = date_col
        self.date_fmt = date_fmt
        
class TimeBasedExtraction(Log):
    """ Vectorize the log sequences into time based windows """
    _new_date_col = "_Date_" # Name of the internal date column used for windowing based on date
    _new_rows_col = "_Rows_" # Name of the internal rows column used for windowing based on date
    
    def __init__(self, logging = True):
        super().__init__(self.__class__.__name__, logging)
        
    def transform(self, x_data, event_col, wp, y_data = None):
        """ Transform raw logs into windows of size `window_size` and step `window_step`
        
        ### Args:
            x_data (DataFrame): log sequence
            event_col (str): name of the column containing the event ids
            wp (WindowParams): parameters for windowing
            y_data (Numpy array): (optional) labels for the log sequence
            
        ### Examples
            Single column "Date": 08/24/2021 use `date_col=["Date"], date_fmt="%m/%d/%Y"`
            Multiple columns "Year", "Month", "Day": 2021, 08, 24 use `date_col=["Year", "Month", "Day"], date_fmt="%Y%m%d"`
            
        ### Notes:
            If `60 * window_step == window_size`, the windows are non-overlapping (fixed windowing)
            
        ### Returns:
            X_df (DataFrame): feature matrix with column names as event ids, each row represents a sliding window
            Y_df (Numpy array): label matrix with column "Label" column containing labels for sliding windows
        """
        x_data, unique_dates = self._str_to_datetimes(x_data, wp.time_col, wp.time_fmt, wp.date_col, wp.date_fmt)
        
        # If dates are provided, create windows for each date
        if unique_dates is not None:
            self.log(f"Dates found: {[x.date() for x in map(pd.to_datetime, unique_dates)]}")
            X_df = pd.DataFrame()
            for date in unique_dates:
                self.log(f"Creating windows for date {pd.to_datetime(date).date()}")
                # Get the events for the given date
                date_events = x_data[x_data[self._new_date_col] == date]
                # Create the windows for the given date
                date_windows = self._create_windows(date_events, wp.window_size, wp.time_col, wp.window_step, event_col)
                X_df = pd.concat([X_df, date_windows], ignore_index=True)
        else:
            # Otherwise create windows for the whole sequence
            X_df = self._create_windows(x_data, wp.window_size, wp.time_col, wp.window_step, event_col)
            
        # Split the data
        if y_data is not None:
            Y = self._split_labels(X_df, y_data)
        else:
            Y = None
        
        return X_df, Y
        
    def _str_to_datetimes(self, x_data, time_col, time_format_str, date_col, date_col_format):
        """ Convert time and date columns into datetime using the specified formats """
        # Convert time into datetime
        x_data[time_col] = pd.to_datetime(x_data[time_col], format=time_format_str)
        unique_dates = None
        
        # Convert date into datetime if specified
        if date_col is not None:
            assert date_col_format is not None, "Date column format must be specified"
            # Date specified in single column
            if len(date_col) == 1:
                x_data[self._new_date_col] = pd.to_datetime(x_data[date_col[0]], format=date_col_format)
            else: # Date specified in multiple columns
                # Merge the date columns into a new single column
                x_data[self._new_date_col] = x_data[date_col].apply(lambda x: "".join(x), axis=1)
                x_data[self._new_date_col] = pd.to_datetime(x_data[self._new_date_col], format=date_col_format)
            # Get all the unique dates
            unique_dates = pd.unique(x_data[self._new_date_col])
            
        return x_data, unique_dates
        
    def _create_windows(self, x_data, window_size, window_col, window_step, event_col):
        """ Create windows of `window_size`, repeating every `window_step` for the given log sequence based on time """
        start_time = x_data[window_col].min()
        end_time = x_data[window_col].max()
        eventIds = OrderedDict()
        
        self.log(f"Window range from {start_time.time()} to {end_time.time()}")
        
        for window_start in pd.date_range(start=start_time, end=end_time, freq=f"{window_step}s"):
            self.log(f"Creating window for {window_start.time()}")
            window_end = window_start + pd.Timedelta(minutes=window_size)
            window = x_data[(x_data[window_col] >= window_start) & (x_data[window_col] < window_end)]
            # Make sure to keep the rows as well, so we can use them for splitting labels
            eventIds[window_start] = (window[event_col].to_list(), window.index.to_list())
        
        # Turn the dictionary into a list of tuples so we can create a dataframe from it
        events = [(time, events, rows) for time, (events, rows) in eventIds.items()]
        
        # Merge the event counts into a dataframe
        X_df = pd.DataFrame(events, columns=["Time", event_col, self._new_rows_col])
        
        return X_df
    
    def _split_labels(self, X_df, y_data):
        # Split the labels based on rows if specified
        if y_data is not None:
            labels_dict = {}
            label_row = 0
            y_data = np.array(y_data)
            for rows in X_df[self._new_rows_col]:
                # If at least one label is "Anomaly", the whole window is labeled as "Anomaly"
                if 1 in y_data[rows]:
                    labels_dict[label_row] = 1
                else:
                    labels_dict[label_row] = 0
                label_row += 1
            # Create array from the dictionary
            y_data = list(labels_dict.values())

        # Count events in each log sequence
        return np.array(y_data)