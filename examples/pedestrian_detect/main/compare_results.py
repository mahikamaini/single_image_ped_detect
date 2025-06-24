from pathlib import Path
import csv

dataset_path = Path("/Users/mahikamaini/esp/single_image_ped_detect/examples/pedestrian_detect/test_set")
positive_folder = dataset_path / "dan"
negative_folder = dataset_path / "empty"
detection_results = Path("detection_results.txt")

results = [["image name", "pedestrian in image", "model prediction"]]

def match(path, name):
    with open(path, "r") as file:
        for line in file:
            if line.startswith("Image: "):
                blocks = line.split(" ")
                full_path = blocks[1]
                img_name = full_path.split("/")[-1]
                if (img_name == name):
                    if "No pedestrian detected" in line:
                        return "FALSE"
                    elif "pedestrian(s) detected" in line:
                        return "TRUE"
    return "Not found"

for img_path in positive_folder.glob("*.JPG"):
    row = [img_path.name, "TRUE", match(detection_results, img_path.name)]
    results.append(row)

for img_path in negative_folder.glob("*.JPG"):
    row = [img_path.name, "FALSE", match(detection_results, img_path.name)]
    results.append(row)

output_path = "ground_truth.csv"
with open(output_path, mode="w", newline="") as file:
    writer = csv.writer(file)
    writer.writerows(results)