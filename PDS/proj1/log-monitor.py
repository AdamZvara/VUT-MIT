"""
log-monitor tool to detect anomalies in log files using LogCluster algorithm

Author: Adam Zvara (xzvara01@stud.fit.vutbr.cz)
Date: 4/2024
"""
import argparse
import json

from src.DataLoader import DataLoader
from src.FeatureExtraction import FeatureExtraction
from src.FeatureExtractionModels.TimeWindow import WindowParams
from src.LogCluster import LogCluster

# Mandatory arguments (required by the project specification)
parser = argparse.ArgumentParser(prog='log-monitor', description='Log monitoring tool')
parser.add_argument('--training', type=str, help='Training log file')
parser.add_argument('--testing',  type=str, help='Testing log file')

# Optional arguments
parser.add_argument('--train_label',  type=str, help='Training labels file')
parser.add_argument('--test_label',   type=str, help='Testing labels file')
parser.add_argument('-c', '--config', type=str, help='Configuration file')

# Knowledge base import/export
parser.add_argument('--import_path', type=str, help='Knowledge base file (instead of training file)')
parser.add_argument('--export_path', type=str, help='Export path for knowledge base')

def parse_config_file(config_file):
    with open(config_file, 'r') as f:
        config = json.load(f)
    return config

def print_usage():
    parser.print_usage()
    exit(1)

def check_valid_args(args):
    # Check if either training data or knowledge base is provided
    if (args.training is None) and (args.import_path is None):
        print("Either training data or knowledge base must be provided")
        print_usage()
        
    if (args.training is not None) and (args.import_path is not None):
        print("Can not use training data with import flag")
        print_usage()
    
    if (args.import_path is not None) and (args.export_path is not None):
        print("Cannot use import and export flags at the same time")
        print_usage()
    
    if (args.training is not None) and (args.config is None):
        print("Configuration file must be provided")
        print_usage()
        
def check_valid_config(config):
    # Check if mandatory configuration parameters are provided
    for i in ["event_col", "windowing", "max_dist", "threshold", "tf_idf", "contrast"]:
        if i not in config.keys():
            print("Missing mandatory configuration parameters")
            exit(1)
    
    # Check if windowing parameters are provided
    if config["windowing"] == "session":
        for i in ["session_reg", "session_col"]:
            if i not in config.keys():
                print("Missing mandatory configuration parameters for session windowing")
                exit(1)
    elif config["windowing"] == "sliding":
        for i in ["window_size", "window_step", "time_col", "time_fmt", "date_col", "date_fmt"]:
            if i not in config.keys():
                print("Missing mandatory configuration parameters for sliding windowing")
                exit(1)
    elif config["windowing"] == "fixed":
        for i in ["window_size", "time_col", "time_fmt", "date_col", "date_fmt"]:
            if i not in config.keys():
                print("Missing mandatory configuration parameters for fixed windowing")
                exit(1)
            
def vectorize(config, data, labels):
    # Load training data
    x_train, y_train = DataLoader().load_csv(data, labels)
        
    if y_train is not None:
        x_train = x_train[y_train == 0]
    
    # Vectorize training data
    feature_extraction = FeatureExtraction(event_col=config["event_col"])
    windowing = config["windowing"]
    
    # Apply windowing
    if windowing == "session":
        x_train, y_train = feature_extraction.session_windowing(x_train, config["session_reg"], config["session_col"], y_train)
    elif windowing == "sliding":
        wparams = WindowParams(config["window_size"], config["window_step"], config["time_col"], config["time_fmt"], config["date_col"], config["date_fmt"])
        x_train, y_train = feature_extraction.sliding_windowing(x_train, wparams, y_train)
    elif windowing == "fixed":
        wparams = WindowParams(config["window_size"], 60 * config["window_size"], config["time_col"], config["time_fmt"], config["date_col"], config["date_fmt"])
        x_train, y_train = feature_extraction.fixed_windowing(x_train, wparams, y_train)
        
    # Apply weighting
    x_train = feature_extraction.apply_weighting(x_train, tf_idf=config["tf_idf"], contrast_w=config["contrast"])
    
    return feature_extraction, x_train, y_train
        
def initialize_model(x_train, y_train, model, fe, export_path):
    # Train the model if no knowledge base is provided
    if y_train is not None:
        model.fit(x_train[y_train == 0]) # Use only normal samples for training
    else:
        model.fit(x_train) # Otherwise use all samples
    
    if export_path is not None:
        model.export_base(export_path, fe)
        
def session_print_anomalies(anomalies, feature_extraction):
    # Print session ids of anomalies
    for i in anomalies.index.values:
        print(feature_extraction.session_ids[i])
    
if __name__ == '__main__':
    # Parse arguments
    args = parser.parse_args()
    check_valid_args(args)
    if args.config is not None:
        config = parse_config_file(args.config)
        check_valid_config(config)
    
    model = None
    
    # Training phase
    if args.import_path is not None:
        # Import knowledge base
        model = LogCluster()
        feature_extraction = model.import_base(args.import_path)
        if args.config != None and config["threshold"] != None:
            model.threshold = config["threshold"]
    else:
        # Otherwise train the model
        model = LogCluster(max_dist=config["max_dist"], threshold=config["threshold"], contrast_w=config["contrast"])
        feature_extraction, x_train, y_train = vectorize(config, args.training, args.train_label)
        initialize_model(x_train, y_train, model, feature_extraction, args.export_path)
    
    # Transform testing data
    if args.testing is not None:
        x_test, y_test = DataLoader().load_csv(args.testing, args.test_label)
        x_test, y_test = feature_extraction.transform(x_test, y_test)
        x_test = feature_extraction.apply_weighting(x_test, feature_extraction.tf_idf, feature_extraction.contrast_w)
        if args.test_label is None:
            anomalies = model.retrieve_anomalies(x_test)
            if (config["windowing"] != "session"):
                print(anomalies)
            else:
                session_print_anomalies(anomalies, feature_extraction)
        else:
            model.evaluate(x_test, y_test)
        