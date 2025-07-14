from pathlib import Path
import csv

dataset_path = Path("/Users/mahikamaini/Downloads/seattleish-camera-traps/camera_trap_images")
# positive_folder = dataset_path / "dan"
# negative_folder = dataset_path / "empty"
detection_results = Path("detection_results copy.txt")
image_list_file = Path("image_list.txt")

results = [["image name", "pedestrian in image", "model prediction"]]

positive_folders = {"dan", "dan_and_dog"}

def match(detection_results, full__img_path):
    with open(detection_results, "r") as file:
        for line in file:
            if line.startswith("Image: "):
                blocks = line.split(" ")
                path = blocks[1]
                if (path == full__img_path):
                    if "No pedestrian detected" in line:
                        return "FALSE"
                    elif "pedestrian(s) detected" in line:
                        return "TRUE"
    return "Not found"

# for img_path in positive_folder.rglob("*.JPG"):
#     row = [img_path.name, "TRUE", match(detection_results, img_path.name)]
#     results.append(row)

# for img_path in negative_folder.rglob("*.JPG"):
#     row = [img_path.name, "FALSE", match(detection_results, img_path.name)]
#     results.append(row)

with open(image_list_file, "r") as f:
    for line in f:
        full_path_str = line.strip()
        full_path = Path(full_path_str)

        img_name = full_path.name
        parent_folder = full_path.parent.name.lower()

        ground_truth = "TRUE" if parent_folder in positive_folders else "FALSE"
        prediction = match(detection_results, full_path_str)

        results.append([img_name, ground_truth, prediction])

output_path = "ground_truth.csv"
with open(output_path, mode="w", newline="") as file:
    writer = csv.writer(file)
    writer.writerows(results)