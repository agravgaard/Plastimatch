function A = readhis(fn)

fn = 'c:/tmp/00007A00.his';

fp = fopen(fn,'r');
if (fp == -1)
  error ('Cannot open file for reading');
end

his.FileType = fread (fp, 1, 'int16');
his.HeaderSize = fread (fp, 1, 'int16');
his.HeaderVersion = fread (fp, 1, 'int16');
his.FileSize = fread (fp, 1, 'uint32');
his.ImageHeaderSize = fread (fp, 1, 'int16');
his.ULX = fread (fp, 1, 'int16');
his.ULY = fread (fp, 1, 'int16');
his.BRX = fread (fp, 1, 'int16');
his.BRY = fread (fp, 1, 'int16');
his.NrOfFrames = fread (fp, 1, 'int16');
his.Correction = fread (fp, 1, 'int16');
his.IntegrationTime = fread (fp, 1, 'double');
his.TypeOfNumbers = fread (fp, 1, 'double');

fseek (fp, 68, 'bof');

his.HeaderID = fread (fp, 1, 'uint8');
his.PROMID = fread (fp, 1, 'uint16');
his.NumberOfEmptyLines = fread (fp, 1, 'uint8');
his.Rows = fread (fp, 1, 'uint16');
his.Columns = fread (fp, 1, 'uint16');

fseek (fp, 68 + 32, 'bof');

rows = his.BRY-his.ULY+1;
cols = his.BRX-his.ULX+1;
A = fread (fp, [cols,rows], 'uint16');
A = A';
fclose(fp);

