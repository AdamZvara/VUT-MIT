#############################################################################################################################
#   project: flp-fun (1st project regarding decision trees to Functional and Logic Programming course at FIT, BUT)
#    author: David Mihola (xmihol00)
#     email: xmihol00@stud.fit.vutbr.cz
#      date: 24. 3. 2024
# file info: Script testing the Haskell project implementation of a decision tree classifier.
#############################################################################################################################

# usage e.g.: python3 test_flp.py -datasets p --test_type training
#           : python3 test_flp.py -datasets p --test_type inference
#           : python3 test_flp.py
#           : python3 test_flp.py
#           : python3 test_flp.py --test_type training
#           : python3 test_flp.py --test_type inference

import argparse
import datetime
import itertools
import os
import time

import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeClassifier
from termcolor import colored
from tqdm import tqdm

datasets = {
    "p": "penguins_all.csv",
    "i": "iris_all.csv",
    "h": "housing_all.csv",
    "w": "wines_all.csv",
    # TODO add some more maybe synthetic datasets to test something specific
}

splits = [0, 0.25, 0.5]  # test/train splits of datasets

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run tests on flp-fun")

    parser.add_argument(
        "--test_type",
        type=str,
        default="both",
        choices=["both", "training", "inference", "b", "t", "i"],
        help="Type of tests to run. Default is both. b for both, t for training, i for inference.",
    )

    parser.add_argument(
        "--datasets",
        type=str,
        nargs="+",
        default=["p", "i", "h", "w"],
        choices=["p", "i", "h", "w"],
        help="Data sets to use for testing. p for penguins, i for iris, h for housing, w for wines. Default is all.",
    )

    parser.add_argument("--seed", "-s", type=int, default=42)
    parser.add_argument("--random", "-r", action="store_true")

    args = parser.parse_args()

    # ensure reproducibility or randomness
    seed = args.seed
    if args.random:
        seed = int(datetime.datetime.now().timestamp())
    np.random.seed(seed)

    # make sure the script will find all the necessary files when run from a different directory
    script_path = os.path.dirname(__file__)
    if not script_path:
        script_path = "."

    strip_string = lambda x: x.strip() if isinstance(x, str) else x  # strip string values in the dataframes

    if args.test_type in ["both", "b", "training", "t"]:
        # remove old output files if they exist
        os.remove("flp-fun_training_stdout.out") if os.path.exists("flp-fun_training_stdout.out") else None
        os.remove("flp-fun_training_stderr.out") if os.path.exists("flp-fun_training_stderr.out") else None
        os.remove("flp-fun_parsing_stderr.out") if os.path.exists("flp-fun_parsing_stderr.out") else None

        # initialize the control variables
        passed_count = 0
        failed_count = 0
        total_count = 0

        results_df = pd.DataFrame(
            columns=[
                "dataset",
                "split",
                "train_accuracy",
                "test_accuracy",
                "reference_train_accuracy",
                "reference_test_accuracy",
                "test_difference",
                "train_time",
            ]
        )

        for dataset, split in tqdm(
            itertools.product([datasets[ds] for ds in args.datasets], splits),
            desc="Running training tests",
            total=len(datasets) * len(splits),
        ):
            df = pd.read_csv(f"{script_path}/datasets/{dataset.strip()}", header=None)

            if split > 0:
                # split the data set to train and test sets
                df_train, df_test = train_test_split(df, test_size=split, random_state=seed)
            else:
                df_train = df.copy()
                df_test = df.copy()

            # save the training split to train a decision tree
            df_train.to_csv("training_data.tmp", index=False, header=False)

            # remove the target column from the data sets
            df_train[df_train.columns[-1]] = df_train[df_train.columns[-1]].apply(strip_string)

            train_y = df_train.pop(df_train.columns[-1]).to_numpy()
            train_X = df_train

            df_test[df_test.columns[-1]] = df_test[df_test.columns[-1]].apply(strip_string)

            test_y = df_test.pop(df_test.columns[-1]).to_numpy()
            test_X = df_test

            # save the test and train sets as samples to be predicted by the trained tree
            train_X.to_csv("train_X.tmp", index=False, header=False)
            test_X.to_csv("test_X.tmp", index=False, header=False)

            # train, evaluate, compare to a reference implementation

            # train the tree and save it
            train_start = time.time()
            os.system(
                f"{script_path}/flp-fun -2 training_data.tmp | tee -a flp-fun_training_stdout.out >trained_tree.tmp 2>flp-fun_training_stderr.out"
            )
            train_time = time.time() - train_start

            # predict the train set, the accuracy should be very high
            os.system(
                f"{script_path}/flp-fun -1 trained_tree.tmp train_X.tmp | tee -a flp-fun_training_stdout.out >predictions.tmp 2>flp-fun_training_stderr.out"
            )

            predictions = pd.read_csv("predictions.tmp", header=None).apply(strip_string).to_numpy().flatten()

            train_accuracy = (predictions == train_y).sum() / train_y.shape[
                0
            ]  # this should be 1.0 for completely overfitted trees on the training set

            # predict the test set
            os.system(
                f"{script_path}/flp-fun -1 trained_tree.tmp test_X.tmp | tee -a flp-fun_training_stdout.out >predictions.tmp 2> flp-fun_training_stderr.out"
            )

            predictions = pd.read_csv("predictions.tmp", header=None).apply(strip_string).to_numpy().flatten()

            test_accuracy = (predictions == test_y).sum() / test_y.shape[0]  # this will differ based on implementation

            # reference implementation
            clf = DecisionTreeClassifier(random_state=seed)
            clf.fit(train_X, train_y)

            clf_predictions_train = clf.predict(train_X)
            clf_predictions_test = clf.predict(test_X)

            reference_train_accuracy = (clf_predictions_train == train_y).sum() / train_y.shape[0]
            reference_test_accuracy = (clf_predictions_test == test_y).sum() / test_y.shape[0]

            # compare prediction with the reference, this can even be negative if the reference implementation is worse.
            test_difference = reference_test_accuracy - test_accuracy

            # add row to dataframe
            results_df = pd.concat(
                [
                    results_df,
                    pd.DataFrame(
                        [
                            [
                                dataset,
                                round(split, 2),
                                train_accuracy,
                                test_accuracy,
                                reference_train_accuracy,
                                reference_test_accuracy,
                                test_difference,
                                train_time,
                            ]
                        ],
                        columns=results_df.columns,
                    ),
                ],
                ignore_index=True,
            )

        results_df.to_csv("training_tests_result.csv", index=False, float_format="% .3f")

        try:
            os.remove("training_data.tmp") if os.path.exists("training_data.tmp") else None
            os.remove("train_X.tmp") if os.path.exists("train_X.tmp") else None
            os.remove("test_X.tmp") if os.path.exists("test_X.tmp") else None
            os.remove("trained_tree.tmp") if os.path.exists("trained_tree.tmp") else None
            os.remove("predictions.tmp") if os.path.exists("predictions.tmp") else None
        except:
            pass

    all_inference_passed = False

    if args.test_type in ["both", "b", "inference", "i"]:
        os.remove("flp-fun_inference_stdout.out") if os.path.exists("flp-fun_inference_stdout.out") else None
        os.remove("flp-fun_inference_stderr.out") if os.path.exists("flp-fun_inference_stderr.out") else None

        all_inference_passed = True
        passed_count = 0
        failed_count = 0
        total_count = 0
        failed = []

        # run inference tests from directories 'trained', 'values' and 'ground_truth', more tests can be added, the file names just need to match
        for dataset in tqdm([datasets[ds] for ds in args.datasets], desc="Running inference tests"):
            tree_file = f"{script_path}/trained/{dataset}"

            # run the inference
            os.system(
                f"{script_path}/flp-fun -1 {tree_file} {script_path}/values/{dataset} | tee -a flp-fun_inference_stdout.out >predictions.tmp 2>flp-fun_inference_stderr.out"
            )

            # load the predictions and ground truth
            predictions = pd.read_csv("predictions.tmp", header=None).apply(strip_string).to_numpy().flatten()

            ground_truth = (
                pd.read_csv(f"{script_path}/ground_truth/{dataset}", header=None)
                .apply(strip_string)
                .to_numpy()
                .flatten()
            )

            # compare the predictions to the ground truth, here any difference is a failure since the inference should be deterministic
            same = np.array_equal(predictions, ground_truth)

            if not same:
                all_inference_passed = False
                failed.append(tree_file)
                failed_count += 1
            else:
                passed_count += 1

            total_count += 1

        if all_inference_passed:
            print(colored(f"All inference tests PASSED ({passed_count}/{total_count}).", "green"))
        else:
            print(colored("Some inference tests FAILED.", "red"))

            for fail in failed:
                print(f"Test failed for file {fail}.")

        with open("inference_tests_summary.txt", "w") as f:
            if all_inference_passed:
                f.write("All tests PASSED.\n")
            else:
                f.write("Some tests FAILED.\n")

            f.write(f"Passed:       {passed_count}/{total_count}\n")
            f.write(f"Failed:       {failed_count}/{total_count}\n")
            f.write(f"Success rate: {passed_count/total_count*100:.2f}%\n")
            f.write(f"Failed rate:  {failed_count/total_count*100:.2f}%\n")

        try:
            os.remove("predictions.tmp") if os.path.exists("predictions.tmp") else None
        except:
            pass
