#                      ISO Image Converter Program        
#                    Copyright (C) 2016 Seunghun Han 
#         at National Security Research Institute of South Korea


# Copyright (c) 2016 Seunghun Han at NSR of South Kora
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


# Encoding Buffer
def Encoding(data):
	start = False
	prevData = 0x00
	count = 0x00
	outputData = ""
	size = len(data)
	for index in range(0, size):
		if (start == False):
			prevData = data[index]
			count = 1
			start = True
		else:
			if ((prevData == data[index]) and (count < 255)):
				count = count + 1
			else:
				outputData += chr(count)
				outputData += prevData
				prevData = data[index]
				count = 1

	# Dump Last Data
	outputData += chr(count)
	outputData += prevData

	return outputData

###############################################################################
#	Encoding
###############################################################################
file = open("autorun.iso", "rb")
data = file.read()
size = len(data)

# Align Sector
if (size % 512 != 0):
	for i in range(512 - (size % 512)):
		data += chr(0x00)

outputFile = open("rle.bin", "wb")
outputLength = "const PROGMEM int g_viSectorLength[] = { "

for index in range(0, (size + 511) / 512):
	outputData = ""
	lastIndex = (index + 1) * 512

	if (lastIndex > size):
		lastIndex = size

	#print "%d %d" % (index * 512, lastIndex)
	outputData = Encoding(data[index * 512:lastIndex])

	#print "length = %d, %d, %d" % (len(outputData), lastIndex - index * 512, 
	#	len(data[index * 512:lastIndex]))

	if (len(outputData) < lastIndex - index * 512):
		print "// Encode"
		outputFile.write(chr(len(outputData) % 256))
		outputFile.write(outputData)

		# Make Arduino File
		sectorNumber = index
		print "const PROGMEM char g_vcSector%d[] = {" % sectorNumber,
		outputLength += "0x%02X, " % len(outputData)
		
		for char in outputData:
			print "0x%02X," % ord(char),
		print "};"

	else:
		print "// Normal"
		outputFile.write(chr(len(data[index * 512:lastIndex]) % 256))
		outputFile.write(data[index * 512:lastIndex])

		# Make Arduino File
		sectorNumber = index
		print "const PROGMEM char g_vcSector%d[] = {" % sectorNumber,
		outputLength += "0x%02X, " % len(data[index * 512:lastIndex])
		
		for char in data[index * 512:lastIndex]:
			print "0x%02X," % ord(char),
		print "};"

# Sector Array
print "\n\nconst PROGMEM char* const g_vcSectors[] = {",
for index in range(0, (size + 511) / 512):
	print "g_vcSector%d," % index,
print "};"	

# Length Array
print outputLength + "};"

# Count
print "const int g_iSectorCount = %d;" % ((size + 511) / 512)

outputFile.close()
