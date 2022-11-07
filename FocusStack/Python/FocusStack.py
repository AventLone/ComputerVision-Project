import numpy as np
import cv2


def doLap(image):
    kernel_size = 3
    blur_size = 3
    blurImg = cv2.GaussianBlur(image, (blur_size, blur_size), 0)
    return cv2.Laplacian(blurImg, cv2.CV_32F, ksize=kernel_size)


class FocusStack:
    def __init__(self, inputImg):
        self.Input = inputImg
        self.alignedImg = []

    def align_images(self):
        detector = cv2.SIFT_create()

        #   We assume that image 0 is the "base" image and align everything to it
        self.alignedImg.append(self.Input[0])
        refeGray = cv2.cvtColor(self.Input[0], cv2.COLOR_BGR2GRAY)
        keypoints_refe, descriptors_refe = detector.detectAndCompute(refeGray, None)
        matcher = cv2.BFMatcher()

        for i in range(1, len(self.Input)):
            keypoints_i, descriptors_i = detector.detectAndCompute(self.Input[i], None)

            pairMatches = matcher.knnMatch(descriptors_i, descriptors_refe, k=2)
            rawMatches = []
            for m, n in pairMatches:
                if m.distance < 0.7 * n.distance:
                    rawMatches.append(m)

            sortMatches = sorted(rawMatches, key=lambda x: x.distance)
            matches = sortMatches[0:128]

            points_i = np.zeros((len(matches), 1, 2), dtype=np.float32)
            points_refe = np.zeros((len(matches), 1, 2), dtype=np.float32)

            for j in range(len(matches)):
                points_i[j] = keypoints_i[matches[j].queryIdx].pt
                points_refe[j] = keypoints_refe[matches[j].trainIdx].pt

            homography, mask = cv2.findHomography(points_i, points_refe, cv2.RANSAC, ransacReprojThreshold=2.0)

            newimage = cv2.warpPerspective(self.Input[i], homography, (self.Input[i].shape[1], self.Input[i].shape[0]),
                                           flags=cv2.INTER_LINEAR)
            self.alignedImg.append(newimage)

    def focus_stack(self):
        self.align_images()
        laps = []
        for img in self.alignedImg:
            laps.append(doLap(cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)))

        laps = np.asarray(laps)

        output = np.zeros(shape=self.alignedImg[0].shape, dtype=self.alignedImg[0].dtype)

        abs_laps = np.absolute(laps)  # 数组中每个元素取绝对值
        maxima = abs_laps.max(axis=0)  # 返回每列元素最大值

        bool_mask = abs_laps == maxima
        mask = bool_mask.astype(np.uint8)

        for i in range(len(self.alignedImg)):
            cv2.bitwise_not(self.alignedImg[i], output, mask=mask[i])

        return 255 - output
