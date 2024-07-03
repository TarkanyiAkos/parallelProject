import threading
import datetime
import time
import operator
from scipy.spatial.distance import cdist
from multiprocessing import Process, Manager
import numpy as np
import json
import os
from PIL import Image
import matplotlib.pyplot as plt
import math

### Result Visualization, provided by University
def crop_images(images, upper, lower):
    cropped_image = []
    for image, top_cord, bottom_cord in zip(images, upper, lower):
        cropped_image.append(image[top_cord[1]:bottom_cord[1], top_cord[0]:bottom_cord[0], :])
    return np.array(cropped_image, dtype=object)


def read_extended_dataset(root_folder='./images/', extended_gt_json='./images/gt_reduced.json', w=60, h=80):
    """
        reads the extended ground truth, returns:
            images: the images in color (80x60x3)
            shape labels: array of strings
            color labels: array of arrays of strings
            upper_left_coord: (x, y) coordinates of the window top left
            lower_right_coord: (x, y) coordinates of the window bottom right
            background: array of booleans indicating if the defined window contains background or not
    """
    ground_truth_extended = json.load(open(extended_gt_json, 'r'))
    img_names, class_labels, color_labels, upper, lower, background = [], [], [], [], [], []

    for k, v in ground_truth_extended.items():
        img_names.append(os.path.join(root_folder, 'train', k))
        class_labels.append(v[0])
        color_labels.append(v[1])
        upper.append(v[2])
        lower.append(v[3])
        background.append(True if v[4] == 1 else False)

    imgs = load_imgs(img_names, w, h, True)

    idxs = np.arange(imgs.shape[0])
    np.random.seed(42)
    np.random.shuffle(idxs)

    imgs = imgs[idxs]
    class_labels = np.array(class_labels)[idxs]
    color_labels = np.array(color_labels, dtype=object)[idxs]
    upper = np.array(upper)[idxs]
    lower = np.array(lower)[idxs]
    background = np.array(background)[idxs]

    return imgs, class_labels, color_labels, upper, lower, background


def read_dataset(root_folder='./images/', gt_json='./test/gt.json', w=60, h=80, with_color=True):
    """
        reads the dataset (train and test), returns the images and labels (class and colors) for both sets
    """
    np.random.seed(123)
    ground_truth = json.load(open(gt_json, 'r'))

    train_img_names, train_class_labels, train_color_labels = [], [], []
    test_img_names, test_class_labels, test_color_labels = [], [], []
    for k, v in ground_truth['train'].items():
        train_img_names.append(os.path.join(root_folder, 'train', k))
        train_class_labels.append(v[0])
        train_color_labels.append(v[1])

    for k, v in ground_truth['test'].items():
        test_img_names.append(os.path.join(root_folder, 'test', k))
        test_class_labels.append(v[0])
        test_color_labels.append(v[1])

    train_imgs = load_imgs(train_img_names, w, h, with_color)
    test_imgs = load_imgs(test_img_names, w, h, with_color)

    np.random.seed(42)

    idxs = np.arange(train_imgs.shape[0])
    np.random.shuffle(idxs)
    train_imgs = train_imgs[idxs]
    train_class_labels = np.array(train_class_labels)[idxs]
    train_color_labels = np.array(train_color_labels, dtype=object)[idxs]

    idxs = np.arange(test_imgs.shape[0])
    np.random.shuffle(idxs)
    test_imgs = test_imgs[idxs]
    test_class_labels = np.array(test_class_labels)[idxs]
    test_color_labels = np.array(test_color_labels, dtype=object)[idxs]

    return train_imgs, train_class_labels, train_color_labels, test_imgs, test_class_labels, test_color_labels


def load_imgs(img_names, w, h, with_color):
    imgs = []
    for tr in img_names:
        imgs.append(read_one_img(tr + '.jpg', w, h, with_color))
    return np.array(imgs)


def read_one_img(img_name, w, h, with_color):
    img = Image.open(img_name)

    if with_color:
        img = img.convert("RGB")
    else:
        img = img.convert("L")

    if img.size != (w, h):
        img = img.resize((w, h))
    return np.array(img)


def visualize_retrieval(imgs, topN, info=None, ok=None, title='', query=None):
    def add_border(color):
        return np.stack(
            [np.pad(imgs[i, :, :, c], 3, mode='constant', constant_values=color[c]) for c in range(3)], axis=2
        )

    columns = 4
    rows = math.ceil(topN/columns)
    if query is not None:
        fig = plt.figure(figsize=(10, 8*6/8))
        columns += 1
        fig.add_subplot(rows, columns, 1+columns)
        plt.imshow(query)
        plt.axis('off')
        plt.title(f'query', fontsize=8)
    else:
        fig = plt.figure(figsize=(8, 8*6/8))

    for i in range(min(topN, len(imgs))):
        sp = i+1
        if query is not None:
            sp = (sp - 1) // (columns-1) + 1 + sp
        fig.add_subplot(rows, columns, sp)
        if ok is not None:
            im = add_border([0, 255, 0] if ok[i] else [255, 0, 0])
        else:
            im = imgs[i]
        plt.imshow(im)
        plt.axis('off')
        if info is not None:
            plt.title(f'{info[i]}', fontsize=8)
    plt.gcf().suptitle(title)
    plt.show()


# Visualize k-mean with 3D plot
def Plot3DCloud(km, rows=1, cols=1, spl_id=1):
    ax = plt.gcf().add_subplot(rows, cols, spl_id, projection='3d')

    for k in range(km.K):
        Xl = km.X[km.labels == k, :]
        ax.scatter(
            Xl[:, 0], Xl[:, 1], Xl[:, 2], marker='.', c=km.centroids[np.ones((Xl.shape[0]), dtype='int') * k, :] / 255
        )

    plt.xlabel('dim 1')
    plt.ylabel('dim 2')
    ax.set_zlabel('dim 3')
    return ax


def visualize_k_means(kmeans, img_shape):
    def prepare_img(x, img_shape):
        x = np.clip(x.astype('uint8'), 0, 255)
        x = x.reshape(img_shape)
        return x

    fig = plt.figure(figsize=(8, 8))

    X_compressed = kmeans.centroids[kmeans.labels]
    X_compressed = prepare_img(X_compressed, img_shape)

    org_img = prepare_img(kmeans.X, img_shape)

    fig.add_subplot(131)
    plt.imshow(org_img)
    plt.title('original')
    plt.axis('off')

    fig.add_subplot(132)
    plt.imshow(X_compressed)
    plt.axis('off')
    plt.title('kmeans')

    Plot3DCloud(kmeans, 1, 3, 3)
    plt.title('núvol de punts')
    plt.show()

### KNN Class
class KNN:
    def __init__(self, train_data, labels):
        self.td_data = False
        self._init_train(train_data)
        self.labels = np.array(labels)
        self.runningProcesses = 0

    def _init_train(self, train_data):
        if hasattr(train_data[0][0][0], "__len__"):
            multi = len(train_data[0][0][0])
            self.td_data = True
        else:
            multi = 1
        self.train_data = train_data.reshape(len(train_data), len(train_data[0]) * len(train_data[0][0]) * multi)
        for i in range(len(self.train_data)):
            for j in range(len(self.train_data[0])):
                self.train_data[i][j] = float(self.train_data[i][j])
    
    def get_k_neighbours(self, test_data, k, threadID, results):
        self.runningProcesses += 1
        imageLabels = self.labels
        arr = test_data.reshape(len(test_data), len(self.train_data[0]))
        distance = np.empty([len(arr), len(self.train_data)])
        cdist(arr, self.train_data, out=distance)
        closests = np.empty([len(arr), k])
        for i in range(len(closests)):
            for j in range(k):
                currentSmallest = min(distance[i])
                csID = 0
                for l in range(len(distance[i])):
                    if distance[i][l] == currentSmallest:
                        csID = l
                        distance[i][l] = max(distance[i])
                        break
                closests[i][j] = int(csID)
        finalMatrix = [["" for _ in range(len(closests[0]))] for _ in range(len(closests))]
        for i in range(len(finalMatrix)):
            for j in range(len(finalMatrix[0])):
                finalMatrix[i][j] = self.labels[int(closests[i][j])]
        results[threadID] = finalMatrix
        self.runningProcesses -= 1

    def get_class(self, neighbors):
        returnArray = np.array([None] * len(neighbors), dtype='<U8')
        for i in range(len(neighbors)):
            elementCounter = [None] * len(neighbors[0])
            for j in range(len(neighbors[0])):
                currItem = neighbors[i][j]
                elementCounter[j] = neighbors[i].count(currItem)
            mostFreqNum = max(elementCounter)
            mostFreqIndex = 0
            for j in range(len(elementCounter)):
                if mostFreqNum == elementCounter[j]:
                    mostFreqIndex = j
                    break
            mce = neighbors[i][mostFreqIndex]
            returnArray[i] = mce
        return returnArray

    
    def predict(self, test_data, k, number_of_threads):
        images_per_process = math.floor(len(test_data) / number_of_threads)
    
        with Manager() as manager:
            results = manager.dict()
            asd = 0
            if number_of_threads == 1:
                self.get_k_neighbours(test_data, k, 0, results)
                neighbors_combined = results[0]
            else:
                proc = [None] * number_of_threads
                for i in range(number_of_threads):
                    firstImg = images_per_process * i
                    lastImg = images_per_process * (i + 1)
                    if i == number_of_threads - 1:
                        lastImg = len(test_data)
                    proc[i] = Process(target=self.get_k_neighbours, args=(test_data[firstImg:lastImg], k, i, results))
                    proc[i].start()
                for i in range(number_of_threads):
                    proc[i].join()
        
                neighbors_combined = []
                for i in range(number_of_threads):
                    neighbors_combined.extend(results[i])
        
            self.neighbors = neighbors_combined
            return self.get_class(neighbors_combined)

###MAIN
if __name__ == '__main__':
    
    # Load all the images and GT
    train_imgs, train_class_labels, train_color_labels, test_imgs, test_class_labels, test_color_labels = read_dataset(root_folder='./images/', gt_json='./images/gt.json')

    # List with all the existent classes
    classes = list(set(list(train_class_labels) + list(test_class_labels)))

    # Load extended ground truth
    imgs, class_labels, color_labels, upper, lower, background = read_extended_dataset()
    cropped_images = crop_images(imgs, upper, lower)

    def Retrieval_by_shape(images, tags, targets):
        #print(f"Number of images: {len(images)}")
        #print(f"Number of tags: {len(tags)}")
        #print(f"Targets: {targets}")
        
        if len(images) != len(tags):
            print(len(tags))
            print(len(images))
            raise ValueError("The length of images and tags must be the same.")
        
        isGood = [False] * len(images)
        for i in range(len(tags)):
            if tags[i] in targets:
                isGood[i] = True

        imageList = [images[i] for i in range(len(isGood)) if isGood[i]]
        return imageList

    def GetClassTags(images, knn, number_of_threads):
        print("Obtaining class tags...")
        returnArray = knn.predict(images, 2, number_of_threads)
        correctedTags = ["Flip Flops" if tag == "Flip Flo" else tag for tag in returnArray]
        print("All class tags obtained successfully!")
        return correctedTags

    #start
    print("Functions loaded successfully. What item should I search for?")
    targetOptions = ["Handbags", "Sandals", "Jeans", "Dresses", "Heels", "Socks", "Flip Flops", "Shirts"]
    print(targetOptions)
    st = input()
    while st not in targetOptions:
        print("ERROR: " + st + " is not a valid item. Please check for typos or provide a different item!")
        st = input()
    searchTargets = [st]

    print("\nNumber of threads (Leave empty for 1): ", end="")
    inpNum = input()
    number_of_threads = int(inpNum) if inpNum else 1

    print(f"\nSearching for [{st}] on [{number_of_threads}] threads.")
    images = test_imgs
    
    print("\n\n__________________________________________\nTraining KNN...")
    knn = KNN(train_imgs, train_class_labels)
    start_time = time.time()
    classtags = GetClassTags(images, knn, number_of_threads)
    print("Class tags found in %s seconds.\n" % (time.time() - start_time))
    matchingImages = Retrieval_by_shape(images, classtags, searchTargets)
    visualize_retrieval(matchingImages, len(matchingImages), title=("Searched: " + str(searchTargets)))
