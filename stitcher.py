# Recursively stitches all csv files in subdirectories into one csv file

import os

fp = open('stitched.csv','w') # Open the stitch file for writing
print 'Stitching files into stitched.csv'

for root, dirs, files in os.walk("."):
    if len(files) > 0:
        if root != ".": # Ignore the current directory, only process subdirectories
            for filename in files: # For each file
                if filename[-4:] == '.CSV': # Check it is a .csv
                    longfilename = root + "\\" + filename # Construct its full filename
                    print 'Appending',longfilename
                    fr = open(longfilename,'r') # Read the .csv file
                    fp.write(fr.read()) # Append it to the stitch file
                    fr.close() # Close the .csv file
                
fp.close() # Close the stitch file

            
    
