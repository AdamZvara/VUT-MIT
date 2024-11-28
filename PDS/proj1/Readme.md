# PDS LOGCLUSTER

LogClustering tool for detecting anomalies in log files.

Author: Adam Zvara (xzvara01)

Date: 04/2024

# Project structure 
```
├── analysis.ipynb                     -- Analysis of the HDFS dataset
├── config                             -- Example configuration files
│   ├── fixed_window.json
│   ├── session_window.json
│   └── sliding_window.json
├── data                               -- Datasets
│   ├── HDFS100k
│   │   ├── log_labels.csv
│   │   └── log_structured.csv
│   └── synthetic
│       ├── log_labels.csv
│       └── log_structured.csv
├── LICENSE
├── log-monitor.py                     -- Main log-monitor file
├── Readme.txt                         -- This file
├── requirements.txt                   -- Requirements
├── src                                -- Source code of LogCluster
│   ├── DataLoader.py
│   ├── FeatureExtractionModels
│   │   ├── SessionWindow.py
│   │   └── TimeWindow.py
│   ├── FeatureExtraction.py
│   ├── LogCluster.py
│   └── utils.py
└── test                               -- Simple tests
│   ├── dummy_data
│   │   ├── labels_structured.csv
│   │   ├── labels_structured_nums.csv
│   │   └── log_structured.csv
│   ├── test_clustering.py
│   ├── test_dataloader.py
│    └── test_features.py
└── xzvara01.pdf                       -- Documentation
```
# Installation

First, install dependencies from requirements.txt:

```
python3.10 -m pip install -r requirements.txt
```

After installing packages you can run the provided tests to ensure that everything
is working properly:

```
python3.10 -m unittest
```

LogCluster tool was tested on python versions 3.11.5 and 3.10.11 (merlin).

# Datasets

The LogCluster tool comes with two HDFS dataset containing 100k and 250k log lines 
(with provided labels), which can be used for testing as well as validation of 
the model. The datasets can be found at `data/`.

Alternatively, a synthetic dataset was created to showcase an example, when LogCluster
can be effective (the dataset was create from normal sessions in HDFS100k dataset 
and manually injected with "nicely" separable anomalous logs as described in 
`analysis.ipynb`)

# Configuration file format

The LogCluster tool was designed to be as flexible as possible, which comes with 
a cost of manually setting it's parameters to cater to different use cases. 
To make the configuration of LogCluster more user-friendly, a configuration file is 
utilized.

The configuration file is in JSON format has the following fields:
MANDATORY FIELDS (each parameter must be explicitly defined)
  - windowing (string) - type of windowing used during feature extraction (values: [session, sliding, fixed])
  - event_col (string) - structured log file column name containing event templates (generated in log parsing phase)
  - max_dist (float)   - maximum distance to group clusters together
  - threshold (float)  - anomaly detection threshold
  - tf_idf (boolean)   - whether to use inverse document frequency weighting
  - contrast (boolean) - whether to use contrast based weighting
    
WINDOWING FIELDS (depending on which windowing is used in `windowing` option)
**Session windowing configuration**
  - session_reg (string) - regex used to group events into sequences based on sessionID
  - session_col (string) - structured log file column name which is used to look for sessionID

**Fixed window configuration**
  - window_size (int)   - size of the window in minutes
  - time_col (string)   - structured log file column name containing the time information
  - time_fmt (string)   - format of the time specified in time_col
  - date_col ([string]) - list of structured lof file column name containing the date
                        - (can be multiple columns, e.g ["Year", "Month", "Day"])
  - date_fmt (string)   - format of the date column(s)

**Sliding window configuration** (same as fixed window with additional parameter)
  - window_step (int) - size of the stride in seconds

Examples of valid configuration files can be found at `config/`.

# Examples

### Model initialization:

The LogCluster model is initialized from the training data provided with the
`--training` parameter. The configuration file is passed via the `--config` parameter.
Since the LogCluster tool is only initialized on normal log sequences, 
there are 2 ways to initialize the model:

i) With only the --training parameter without `--train_label` - this configuration
assumes that the whole training dataset contains only normal sequences and therefore
the model is initialized with the whole dataset.

```
python3.10 log-monitor.py --training data/HDFS100k/log_structured.csv --config config/session_window.json
```

ii) If the training dataset contains anomalies, the label file must be provided with
the `--train_label` parameter, so the LogCluster tool can only train on normal logs.

```
python3.10 log-monitor.py --training data/HDFS100k/log_structured.csv --train_label data/HDFS100k/log_labels.csv --config config/session_window.json\
```

### Anomaly detection:

The target file is passed via the `--testing` parameter and the evaluation
can be done in two different ways:

i) When only `--testing` parameter is provided, log-monitor outputs the anomalous
sequences (please only use for the session windows, time windowing prints out
weighted log sequences as it hasn't been proven usefull).

```
python3.10 log-monitor.py --training data/HDFS100k/log_structured.csv --train_label data/HDFS100k/log_labels.csv --config config/session_window.json --testing data/HDFS100k/log_structured.csv
```

ii) If the `--test_labels` file is provided, the log-monitor tool
prints out precision, recall and F1 measure.

```
python3.10 log-monitor.py --training data/HDFS100k/log_structured.csv --train_label data/HDFS100k/log_labels.csv --config config/session_window.json --testing data/HDFS100k/log_structured.csv --test_label data/HDFS100k/log_labels.csv
```

### Importing and exporting knowledge base

It is possible to train the LogCluster model and save it for later
use with the `--export_path` parameter

```
python3.10 log-monitor.py --training data/HDFS100k/log_structured.csv --train_label data/HDFS100k/log_labels.csv --config config/session_window.json --export_path base
```

It is also possible to import previously generated knowledge base. In this
example, the windowing type and parameters are imported as well, so that 
the testing sequences are vectorized in the same way as the training sequences.
Therefore, using this configuration, you can only change the `threshold` in the
configuration file to change the behaviour of the LogCluster model. 

```
python3.10 log-monitor.py --import_path base --config config/session_window.json  --testing data/HDFS100k/log_structured.csv --test_label data/HDFS100k/log_labels.csv
```
