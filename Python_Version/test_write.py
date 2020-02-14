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
