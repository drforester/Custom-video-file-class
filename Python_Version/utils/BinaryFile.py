'''
A Python class implementation of a binary file reader & writer
'''
import logging
import struct
import numpy as np
from os.path import getsize

FILE_HEADER_SIZE = 1024 # size in bytes
NBYTES_DICT = {'uint8':1, 'uint16':2, 'uint32':4, 'float32':4, 'float64':8}

class BinaryFile:

    def __init__(self, filename):
        self.filePtr = None
        self.filestr = filename
        self.imgDataType = ""
        self.imgH = 1
        self.imgW = 1
        self.depth = 1
        self.frhsize = 0
        self.metasize = 0
        self.nbytes = 0
        self.totalFrames = 1
        self.offset = 0
        self.currFrame = np.zeros((self.imgW, self.imgH, self.depth))
        self.Img = np.zeros((self.imgW, self.imgH, self.depth))
        self.FrameHeader = ""
        self.MetaHeader = ""
        
    
    def read_file_header(self, istart, iend, verbose):
        
        infile = self.filestr
        # Open file
        try:
            self.filePtr = open(infile, 'rb')
        except IOError as err:
            logging.debug("Could not open input file %s" % (infile))

        filesize = getsize(infile)
        if verbose: print('filesize =', filesize)

        # Read in header, then rest of binary file
        file_header_dict = {}
        with open(infile, 'rb') as f:
            header = f.read(FILE_HEADER_SIZE)
            #print(header)
            #print('')
            hList = header.decode('utf-8').split('\n')
        
        
        # Read Data Type
        fnfmt = [x.split('FILE_NUMERICAL_FORMAT=')[-1].split('\r')[0]
                     for x in hList
                     if x.startswith('FILE_NUMERICAL_FORMAT=')]
        if len(fnfmt)>0:
            imgDataType = fnfmt[0]
        else: # if not readable, assume uint16
            imgDataType = 'uint16'

        if imgDataType in ['uint8', 'UINT8']:
            imgDataType = 'uint8'
        if imgDataType in ['FLOAT', 'float', 'float32', 'Float32']:
            imgDataType = 'float32'
        if imgDataType in ['FLOAT64', 'float64', 'Float64']:
            imgDataType = 'float64'

        # Read Image Height
        imgHeight = [x.split('IMAGE_HEIGHT=')[-1].split('\r')[0]
                     for x in hList
                     if x.startswith('IMAGE_HEIGHT=')]
        if len(imgHeight)>0:
            imgH = int(imgHeight[0])
        else:
            imgH = 0

        # Read Image Width
        imgWidth = [x.split('IMAGE_WIDTH=')[-1].split('\r')[0]
                     for x in hList
                     if x.startswith('IMAGE_WIDTH=')]
        if len(imgWidth)>0:
            imgW = int(imgWidth[0])
        else:
            imgW = 0

        # Read Image Depth
        imgDepth = [x.split('IMAGE_DEPTH=')[-1].split('\r')[0]
                     for x in hList
                     if x.startswith('IMAGE_DEPTH=')]
        if len(imgDepth)>0:
            depth = int(imgDepth[0])
        else:
            depth = 1 # defaults to 1 color layer


        # Read FRAME_HEADER_SIZE in bytes
        frhsize = [x.split('FRAME_HEADER_SIZE=')[-1].split('\r')[0]
                     for x in hList
                     if x.startswith('FRAME_HEADER_SIZE=')]
        if len(frhsize)>0:
            frhsize = int(frhsize[0])
        else:
            frhsize = 0

        # Read META_HEADER_SIZE in bytes
        metasize = [x.split('META_HEADER_SIZE=')[-1].split('\r')[0]
                     for x in hList
                     if x.startswith('META_HEADER_SIZE=')]
        if len(metasize)>0:
            metasize = int(metasize[0])
        else:
            metasize = 0
            
        nbytes = NBYTES_DICT[imgDataType]
        if verbose: print('filesize', 'imgW', 'imgH', 'depth', 'nbytes', 'frhsize', 'metasize')
        if verbose: print(filesize, imgW, imgH, depth, nbytes, frhsize, metasize)
        totalFrames = int( (filesize - FILE_HEADER_SIZE)/((imgH*imgW*depth*nbytes)+frhsize+metasize) )
        if verbose: print('total frames:', totalFrames)
        
        self.imgDataType = imgDataType
        self.imgH = imgH
        self.imgW = imgW
        self.depth = depth
        self.frhsize = frhsize
        self.metasize = metasize
        self.nbytes = nbytes
        self.totalFrames = totalFrames
        self.frameCount = 0
        self.istart = 0
        self.iend = totalFrames
        
        if istart < totalFrames:
            self.istart = istart
            
        if iend == -1:
            self.iend = self.totalFrames
        elif iend <= self.totalFrames:
            self.iend = iend
        
        # advance file pointer offset past the file header
        self.offset += FILE_HEADER_SIZE
        
        # advance file pointer past istart number of frames
        for j in range(self.istart-1):
            self.offset += self.frhsize
            self.offset += self.metasize
            self.offset += self.imgH * self.imgW * self.depth * self.nbytes
            self.frameCount += 1
            
        
        
    def read_and_advance_fp(self):
        
        hdr_dtype = np.ubyte #np.ubyte
        
        # read frame header if it exists
        if self.frhsize > 0:
            self.FrameHeader = np.memmap(self.filePtr, dtype=hdr_dtype, mode='r', offset=self.offset, shape=self.frhsize).tolist()
            self.FrameHeader = bytes(self.FrameHeader.copy()).decode('utf-8')
            self.FrameHeader = self.FrameHeader.split("  ")[0]
            self.FrameHeader = self.FrameHeader.split("\r\n")[0]
            self.offset += self.frhsize
        else:
            self.FrameHeader = ""
            
        # read meta header if it exists
        if self.metasize > 0:
            self.MetaHeader = np.memmap(self.filePtr, dtype=hdr_dtype, mode='r', offset=self.offset, shape=self.metasize).tolist()
            self.MetaHeader = bytes(self.MetaHeader.copy()).decode('utf-8')
            self.MetaHeader = self.MetaHeader.split("  ")[0]
            self.MetaHeader = self.MetaHeader.split("\r\n")[0]
            self.offset += self.metasize
        else:
            self.MetaHeader = ""
            
        # read the image data
        self.currFrame = np.memmap(self.filePtr, dtype=self.imgDataType, mode='r', offset=self.offset,
                                   shape=(self.imgH * self.imgW * self.depth))
                                   
        if self.depth > 1:
            self.currFrame = self.currFrame.reshape(self.imgH, self.imgW, self.depth)
        else:
            self.currFrame = self.currFrame.reshape(self.imgH, self.imgW)
        
        # advance the file pointer
        self.offset += self.imgH * self.imgW * self.depth * self.nbytes
        self.Img = self.currFrame.copy()
                                   
        if self.frameCount == self.iend:
            self.filePtr.close()
            
        return (self.FrameHeader, self.MetaHeader, self.Img)
        
        
        
    def write_file_header(self, settings):
        
        self.frame_header_size = int(settings["frame_header_size"])
        self.meta_header_size  = int(settings["meta_header_size"])
        self.rows = int(settings["image_height"])
        self.cols = int(settings["image_width"])
        
        if settings["file_numerical_format"] == 'uint8':
            self.fmt = str(self.rows * self.cols)+'B'
        elif settings["file_numerical_format"] == 'uint16':
            self.fmt = str(self.rows * self.cols)+'H'
        else:
            self.fmt = str(self.rows * self.cols)+'I'
            print("file format "+settings["file_numerical_format"]+"not recognized by writer.")     
        
        file_header = str.encode(
                                 '0000000999\r\n' +
                                 'FILE_DATA_BEGIN=0000001024\r\n' +
                                 'CAMERA_BIT_DEPTH='+settings["camera_bit_depth"]+'\r\n' +
                                 'CAMERA_FRAME_RATE='+settings["camera_frame_rate"]+'\r\n' +
                                 'FILE_NUMERICAL_FORMAT='+settings["file_numerical_format"]+'\r\n' +
                                 'FRAME_HEADER_SIZE='+settings["frame_header_size"]+'\r\n' +
                                 'META_HEADER_SIZE='+settings["meta_header_size"]+'\r\n' +
                                 'IMAGE_HEIGHT='+settings["image_height"]+'\r\n' +
                                 'IMAGE_WIDTH='+settings["image_width"]+'\r\n'
                                )
        outfile = self.filestr
        # Open file
        try:
            self.filePtr = open(outfile, 'wb')
        except IOError as err:
            logging.debug("Could not open input file %s" % (outfile))
            
        self.filePtr.write(file_header)
        remaining = FILE_HEADER_SIZE - len(file_header)
        padding = str.encode(' '*remaining)
        self.filePtr.write(padding)
        
        
    
    def write_frame_and_advance_fp(self, isLast, frameHeader, metaHeader, Image):
        
        if frameHeader:
            frame_header = str.encode(frameHeader)
            self.filePtr.write(frame_header)
            remaining = self.frame_header_size - len(frame_header)
            padding = str.encode(' '*remaining)
            self.filePtr.write(padding)
            
        if frameHeader:
            meta_header = str.encode(metaHeader)
            self.filePtr.write(meta_header)
            remaining = self.meta_header_size - len(meta_header)
            padding = str.encode(' '*remaining)
            self.filePtr.write(padding)
            
        frame = np.reshape(Image, self.rows*self.cols)        
        binframe = struct.pack(self.fmt, *frame)
        self.filePtr.write(binframe)
        
        if isLast:
            self.filePtr.close()
