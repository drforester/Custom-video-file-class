## Reading and Writing Custom Binary Files in Python and C++ 

This binary file class implementation supports the writing and reading of frames, i.e. arrays of numbers, and their associated frame headers, i.e. metadata describing the individual image frames, such as time, gps location, etc. The frames and headers are encapsulated in a single binary file which begins with a header which holds information such as the datatype, information about the camera which captured the data, and frame size.  

The images are 2D arrays which are flattened before writing to file. In the Python implementation, the images are NumPy arrays. In the C++ implementation, the images are OpenCV Mat objects.  

Below is an example showing how to use the Python writer to create a binary file with headers and data, followed by an example showing how to read in the binary file that was written.  
The C++_Version directory contains corresponding write and read examples.  


#### Write Example in Python

```
import numpy as np
from utils.BinaryFile import BinaryFile


# replace with the path to your Binary file
out_file_str = "./DATA/testOut.raw"

# Instantiate the BinaryFile object
bfo = BinaryFile(out_file_str)

# information to write to the file header
settings = {}
settings["camera_frame_rate"] = '30'
settings["camera_bit_depth"] = '14'
settings["file_numerical_format"] = 'uint16'
settings["frame_header_size"] = '26'
settings["meta_header_size"] = '32'
settings["image_height"] = '720' 
settings["image_width"] = '1280'

''' Write the file header (advances the fp past the header and leaves the file object open) ----'''
bfo.write_file_header(settings)

''' Write the current frame header, meta header, and image data --------------------------------'''
ii = 0
end_frame = 3
isLast = False
while(ii < end_frame):
    imgnumb = str(ii).zfill(3)
    print(imgnumb)
    
    # write this frame's frame-header
    frameHeader = 'TimeStamp=2030_07_15 '+imgnumb+'\r\n'
    
    # write this frame's meta-header
    metaHeader = 'test_meta_header_info '+imgnumb+'\r\n'
    
    # write this frame's image data
    Image = np.random.randint(0,65535, (1280,720), np.uint16)
    print(Image.shape, Image.dtype)
    
    if ii == (end_frame - 1):
        isLast = True

    # read the current frame headers and data. "isLast" determines whether to close the file
    bfo.write_frame_and_advance_fp(isLast, frameHeader, metaHeader, Image)
    
    ii += 1
```


#### Read Example in Python

```
import numpy as np
from utils.BinaryFile import BinaryFile

# replace with the path to your Grave file
in_file_str = "./DATA/testOut.raw"


''' Instantiate the BinaryFile object ----------------------------------------------------------'''
bfi = BinaryFile(in_file_str)

start_frame = 0 # the index of the first frame to read
end_frame = -1  # the index of the last frame to read (-1 means read all frames)
bfi.read_file_header(start_frame, end_frame, verbose=False)

''' Read the current frame header, meta header, and image data ---------------------------------'''
ii = start_frame
while(ii < bfi.iend):
    imgnumb = str(ii).zfill(3)
    
    frameHdr, metaHdr, Img = bfi.read_and_advance_fp()
    print("\nFrame " + imgnumb)
    print("Frame header     : ", frameHdr)
    print("Meta header      : ", metaHdr)
    print("Image mean value :", f'{np.mean(Img):6.2f}')
    
    ii += 1
```


Both of these Python examples are in the repository, as well as corresponding C++ versions. 

Makefiles are included for building the C++ reader and writer with GCC. To build the writer and reader on your system, you will need to alter OPENCV\_LIB\_PATH and OPENCV\_INC\_PATH to match the OpenCV directories on your system.  

To build the C++ writer:
`~$ make -f Makefile.writer`  

To build the C++ reader:
`~$ make -f Makefile.reader`  
