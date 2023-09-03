# zlib for R
[![Tests](https://github.com/sgeist-ionos/R-zlib/actions/workflows/test.yml/badge.svg?branch=main)](https://github.com/sgeist-ionos/R-zlib/actions/workflows/test.yml)

[![Build and Deploy R Markdown Docs](https://github.com/sgeist-ionos/R-zlib/actions/workflows/build-docs.yml/badge.svg?branch=main)](https://sgeist-ionos.github.io/R-zlib/)

[![Build tarball artifact on release](https://github.com/sgeist-ionos/R-zlib/actions/workflows/build.yml/badge.svg)](https://github.com/sgeist-ionos/R-zlib/actions/workflows/build.yml)

[![codecov](https://codecov.io/gh/sgeist-ionos/R-zlib/graph/badge.svg?token=WRBNXIMB3N)](https://codecov.io/gh/sgeist-ionos/R-zlib)

## Description

The `zlib` package for R aims to offer an R-based equivalent of Python's built-in `zlib` module for data compression and decompression. This package provides a suite of functions for working with zlib compression, including utilities for compressing and decompressing data streams, manipulating compressed files, and working with `gzip`, `zlib` and deflate formats.

## Usage

This example demonstrates how to use the `zlib` Rcpp module for chunked compression and decompression. We will take a string, write it to a temporary file, and then read it back into a raw vector. Then we will compress and decompress the data using the `zlib` Rcpp module.

### Installation

To install the `zlib` package, you can use the following command:

```R  
install.packages("zlib")  # Uncomment this line if zlib is hosted on CRAN or a similar repo```  

### Code Example  

First, make sure to load the `zlib` package:  

```R  
library(zlib)  
```  

#### Writing and Reading a String to/from a Temporary File

```R  
# Create a temporary file  
temp_file <- tempfile(fileext = ".txt")  
  
# Generate example data and write to the temp file  
example_data <- "This is an example string. It contains more than just 'hello, world!'"  
writeBin(charToRaw(example_data), temp_file)  
  
# Read data from the temp file into a raw vector  
file_con <- file(temp_file, "rb")  
raw_data <- readBin(file_con, "raw", file.info(temp_file)$size)  
close(file_con)  
```  

#### Compressing the Data

```R  
# Create a Compressor object  
compressor <- zlib$compressobj(zlib$Z_DEFAULT_COMPRESSION, zlib$DEFLATED, zlib$MAX_WBITS + 16)  
  
# Initialize variables for chunked compression  
chunk_size <- 1024  
compressed_data <- raw(0)  
  
# Compress the data in chunks  
for (i in seq(1, length(raw_data), by = chunk_size)) {  
   chunk <- raw_data[i:min(i + chunk_size - 1, length(raw_data))]  
   compressed_chunk <- compressor$compress(chunk)
   compressed_data <- c(compressed_data, compressed_chunk)
}  
  
# Flush the compressor buffer  
compressed_data <- c(compressed_data, compressor$flush())  
```  

#### Decompressing the Data

```R  
# Create a Decompressor object  
decompressor <- zlib$decompressobj(zlib$MAX_WBITS + 16)  
  
# Initialize variable for decompressed data  
decompressed_data <- raw(0)  
  
# Decompress the data in chunks  
for (i in seq(1, length(compressed_data), by = chunk_size)) {  
  chunk <- compressed_data[i:min(i + chunk_size - 1, length(compressed_data))]  
   decompressed_chunk <- decompressor$decompress(chunk)  
   decompressed_data <- c(decompressed_data, decompressed_chunk)
}  
  
# Flush the decompressor buffer  
decompressed_data <- c(decompressed_data, decompressor$flush())  
```  

#### Verifying the Results

```R  
# Convert decompressed raw vector back to string  
decompressed_str <- rawToChar(decompressed_data)  
  
# Should print TRUE  
print(decompressed_str == example_data)  
```  

By following these steps, you can successfully compress and decompress data in chunks using the `zlib` Rcpp module.

## Why Another Zlib Package for R?

Hello, and welcome to my zlib package! You might wonder, "Why do we need another zlib package when R already has built-in methods for compression and decompression?" Let me clarify why I decided to develop this package.

#### The Problems with Built-in Methods

R's built-in functions like `memDecompress` and `memCompress` are good for simple tasks, but they lack robustness and flexibility for advanced use-cases:

1. **Handling Corrupt Data**: Functions like `memDecompress` are unstable when dealing with gzip bytes that may be corrupt. If the data has multiple header blocks or too small a chunk, the function doesn't just fail—it can even crash your computer.
   *Incomplete Data Stream*: A too-small chunk can hang your system when using
   ```R  
      compressed_data <- memCompress(charToRaw(paste0(rep("This is an example string. It contains more than just 'hello, world!'", 1000), collapse = ", ")))  
      rawToChar(memDecompress(compressed_data[1:300], type="gzip"))    # Caused a hang-up    
      readLines(gzcon(rawConnection(compressed_data[1:300])))    
      # Warnung in readLines(gzcon(rawConnection(compressed_data[1:300])))    
      # unvollständige letzte Zeile in 'gzcon(compressed_data[1:300])' gefunden    
      # [1] "x\x9c\...."  
   ```  
   *Multiple Header Blocks*: R's `memDecompress` doesn't handle gzip data with multiple headers well.
   ```R  
      multi_header_compressed_data_15wbits <- c(memCompress(charToRaw("Hello World"), type="gzip"),memCompress(charToRaw("Hello World"), type="gzip"))  
      rawToChar(memDecompress(multi_header_compressed_data_15wbits, type="gzip"))  # Returns only one compressed header  
   ```
   Whereas, using `pigz` or `gzip` with pipes returns the expected concatenated strings:
   ```R  
      tmp_file <- tempfile(fileext=".gzip")  
      writeBin(multi_header_compressed_data_15wbits, tmp_file)   
      readLines(pipe(sprintf("pigz -c -d %s --verbose 2>/dev/null", tmp_file), open = "rb")) # working correct but with warning   
      # Warnung in readLines(pipe(sprintf("pigz -c -d %s --verbose 2>/dev/null",   
      # unvollständige letzte Zeile in 'pigz -c -d /tmp/Rtmp5IRHIZ/file44665a0e2eb2.gzip --verbose 2>/dev/null' gefunden   
      # [1] "Hello WorldHello World"   
      readLines(pipe(sprintf("gzip -d %s --verbose --stdout", tmp_file), open = "rb")) # not working becouse of the wrong wbits (see next point)   
      # character(0)  
   ```

2. **GZIP File Format Specification**: R's `memCompress` doesn't adhere strictly to the GZIP File Format Specification, particularly regarding the usage of window bits.

   ```R  
      memCompress("Hello World", type="gzip")  # Incorrect 15 wbits  
      # [1] 78 9c f3 48 cd c9 c9 57 08 cf 2f ca 49 01 00 18 0b 04 1d  
   ```

   [Official GZIP File Format Specification](https://www.ietf.org/rfc/rfc1952.txt)

3. **Incorrect Behavior with Different `wbits`**: The behavior of `memCompress` is inconsistent when different `wbits` are used for compression and decompression.

   ```R  
      compressor <- zlib$compressobj(zlib$Z_DEFAULT_COMPRESSION, zlib$DEFLATED, zlib$MAX_WBITS + 16)  
      multi_header_compressed_data_31wbits <- c(c(compressor$compress(charToRaw("Hello World")), compressor$flush()), c(compressor$compress(charToRaw("Hello World")), compressor$flush()))    
      readLines(gzcon(rawConnection(multi_header_compressed_data_31wbits))) # returns a single line if the gzip wbits are correct!      
      # Warnung in readLines(gzcon(rawConnection(wrong_compressed_data_31wbit)))      
      # unvollständige letzte Zeile in 'gzcon(wrong_compressed_data_31wbit)' gefunden      
      # [1] "Hello World"    
      readLines(gzcon(rawConnection(multi_header_compressed_data_15wbits)))      
      # Warnung in readLines(gzcon(rawConnection(wrong_compressed_data)))  
      # Zeile 1 scheint ein nul Zeichen zu enthalten      
      # Warnung in readLines(gzcon(rawConnection(wrong_compressed_data)))      
      # unvollständige letzte Zeile in 'gzcon(wrong_compressed_data)' gefunden      
      # [1] "x\x9c\xf3H\xcd\xc9\xc9W\b\xcf/\xcaI\001"    
      tmp_file <- tempfile(fileext=".gzip")   
      writeBin(multi_header_compressed_data_31wbits, tmp_file)    
      readLines(pipe(sprintf("gzip -d %s --verbose --stdout", tmp_file), open = "rb")) # gzip pipe works with correct wbits      
      # Warnung in readLines(pipe(sprintf("gzip -d %s --verbose --stdout", tmp_file),      
      # unvollständige letzte Zeile in 'gzip -d /tmp/RtmpPIZPMP/file6eed29844dc4.gzip --verbose --stdout' gefunden      
      # [1] "Hello WorldHello World"  
   ```

4. **No Streaming Support**: There's no native way to handle Gzip streams from REST APIs or other data streams without creating temporary files or implementing cumbersome workarounds (e.g. with pipes and tmp files).

#### What My Package Offers

1. **Robustness**: Built to handle even corrupted or incomplete gzip data efficiently without causing system failures.

   ```R  
      compressed_data <- memCompress(charToRaw(paste0(rep("This is an example string. It contains more than just 'hello, world!'", 1000), collapse = ", ")))  
      decompressor <- zlib$decompressobj(zlib$MAX_WBITS)    
      rawToChar(c(decompressor$decompress(compressed_data[1:300]), decompressor$flush()))  # Still working  
   ```

2. **Compliance**: Strict adherence to the GZIP File Format Specification, ensuring compatibility across systems.

   ```R  
      compressor <- zlib$compressobj(zlib$Z_DEFAULT_COMPRESSION, zlib$DEFLATED, zlib$MAX_WBITS + 16)  
      c(compressor$compress(charToRaw("Hello World")), compressor$flush())  # Correct 31 wbits    
      # [1] 1f 8b 08 00 00 00 00 00 00 03 f3 48 cd c9 c9 57 08 cf 2f ca 49 01 00 56 b1 17 4a 0b 00 00 00  
   ```

3. **Flexibility**: Ability to manage Gzip streams from REST APIs without the need for temporary files or other workarounds.


In summary, while R’s built-in methods could someday catch up in functionality, my zlib package for now fills an important gap by providing a more robust and flexible way to handle compression and decompression tasks.

## Future Enhancements

We've identified some exciting opportunities for extending the capabilities of this library. While these features are not currently planned for immediate development, we're open to collaboration or feature requests to bring these ideas to life.

### [Gztool](https://github.com/circulosmeos/gztool)

Gztool specializes in indexing, compressing, and data retrieval for GZIP files. With Gztool integration, you could create lightweight indexes for your gzipped files, enabling you to extract data more quickly and randomly. This would eliminate the need to decompress large gzip files entirely just to access specific data at the end of the file.

### [Pugz](https://github.com/Piezoid/pugz)

Pugz offers parallel decompression of gzipped text files. It employs a truly parallel algorithm that works in two passes, significantly accelerating the decompression process. This could be a valuable addition for users dealing with large datasets and seeking more efficient data processing.

If any of these feature enhancements interest you, or if you have other suggestions for improving the library, feel free to reach out for collaboration.

## Dependencies

### Software Requirements

- [CMake](https://cmake.org/) (version >= 3.10)
- [Ninja](https://ninja-build.org/) build system
- [R](https://www.r-project.org/) (version >= 4.0)
- C++ Compiler (GCC, Clang, etc.)

### Libraries

- zlib

## Development

### Installing Dependencies on Ubuntu

```bash  
sudo apt-get updatesudo apt-get install cmake ninja-build r-base libblas-dev liblapack-dev build-essential 
```  
  
### Installing Dependencies on Red Hat  
  
```bash  
sudo yum updatesudo yum install cmake ninja-build R libblas-devel liblapack-devel gcc-c++
```  
  
### Building  
  
1. Clone the repository:  
    ```bash  
    git clone https://github.com/yourusername/zlib.git  
    ```  
2. Install the package local  
   ```bash  
   make install   
   ```  
3. Build the package local  
   ```bash  
   make build   
   ``` 
    
## License  
  
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.