import numpy as np
from utils.BinaryFile import BinaryFile

# replace with the path to your Grave file
in_file_str = "./DATA/testOut.raw"


''' Instantiate the GraveFile objects ----------------------------------------------------------'''
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
