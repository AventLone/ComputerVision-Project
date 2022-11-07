from FocusStack import *
import cv2

if __name__ == "__main__":
    Images = [cv2.imread('./Images/step0.jpg'), cv2.imread('./Images/step1.jpg'),
              cv2.imread('./Images/step2.jpg'), cv2.imread('./Images/step3.jpg'),
              cv2.imread('./Images/step4.jpg'), cv2.imread('./Images/step5.jpg')]
    # Images = [cv2.imread('./Images/001.png'), cv2.imread('./Images/002.png')]
    a = FocusStack(Images)
    answer = a.focus_stack()
    # # cv2.imwrite('./output.png', answer)
    # cv2.imshow('Outcome2', answer)
    # cv2.waitKey(0)
    # img = cv2.imread("./Images/002.png", 0)
    # cv2.imshow('1', img)
    # cv2.imshow('2', -img)
    # cv2.waitKey(0)
