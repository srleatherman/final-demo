global angle
global mode
angle=90
mode='0'
address = 0x4
from picamera.array import PiRGBArray
from picamera import PiCamera 
import time
import numpy as np
import cv2 as cv
import smbus
camera = PiCamera()
camera.resolution = (640, 480)
camera.framerate = 2

raw_capture = PiRGBArray(camera, size=(640, 480))
bus = smbus.SMBus(1)
time.sleep(0.1)

def writeNumber(value):
    bus.write_byte_data(address, value,angle)
# bus.write_byte_data(address, 0, value)
    return -1

 
# Capture frames continuously from the camera
for frame in camera.capture_continuous(raw_capture, format="bgr", use_video_port=True):
   
     
    # Grab the raw NumPy array representing the image
    image = frame.array
     
    img_hsv = cv.cvtColor(image, cv.COLOR_BGR2HSV)
    #Upper and lower limits for color detection
    red_lower = np.array([90,100,0])
    red_upper = np.array([110,255,255])
    #Isolating red
    mask = cv.inRange(img_hsv, red_lower, red_upper)
    #Applying filters and effects to sharpen mask
    imgmanip = cv.bitwise_and(img_hsv, img_hsv, mask = mask)
    imgmanip = cv.cvtColor(imgmanip, cv.COLOR_HSV2BGR)
    imgmanip = cv.cvtColor(imgmanip, cv.COLOR_BGR2GRAY)
    imgmanip = cv.GaussianBlur(imgmanip, (5,5), 0)
    imgmanip = cv.threshold(imgmanip, 30, 255, cv.THRESH_BINARY)[1]
    #Detecting contours
    cnts = cv.findContours(imgmanip.copy(), cv.RETR_LIST,cv.CHAIN_APPROX_SIMPLE)[1]
    image_copy=image.copy()
    cX = 0
    cY = 0
    cYMin = 0
    cYMinInt = 0
    contour = 0
    if(len(cnts) > 0):
        contour = max(cnts, key=cv.contourArea)
        M = cv.moments(contour)
        if M["m00"] != 0:
            cX = int(M["m10"] / M["m00"])
            cY = int(M["m01"] / M["m00"])
            angle=int((60/2)*((cX-320)/(640-320)))
            cYMin = tuple(contour[contour[:, :, 1].argmax()][0])
            cYMax = tuple(contour[contour[:, :, 1].argmin()][0])
            cYMinInt = int(cYMin[1])
            cYMaxInt = int(cYMax[0])
        cv.circle(image_copy, cYMin, 8, (255,255,0), -1)
        cv.drawContours(image=image_copy, contours=cnts, contourIdx=-1, color=(0,255,0), thickness=2, lineType=cv.LINE_AA)
    cv.imshow('image', image_copy)
     
    # Wait for keyPress for 1 millisecond
    key = cv.waitKey(1) & 0xFF
     
    # Clear the stream in preparation for the next frame
    raw_capture.truncate(0)
     
# If the `q` key was pressed, break from the loop
    if (cX == 0 and cY == 0) or cv.contourArea(contour) < 1000:
        mode=0
    elif abs(angle)<10:
        if(cYMinInt>460):
            mode=3
        else:
            mode=2
    else:  # retuns the quad number and displays the location
        mode=1
        send=str(angle)
    
    if key == ord("q"):
        break
    
    writeNumber(mode)
    print('mode: ',mode)
    print(angle)
    
