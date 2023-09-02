# zlib for R
[![Tests](https://github.com/sgeist-ionos/R-zlib/actions/workflows/test.yml/badge.svg?branch=main)](https://github.com/sgeist-ionos/R-zlib/actions/workflows/test.yml)

[![Build and Deploy R Markdown Docs](https://github.com/sgeist-ionos/R-zlib/actions/workflows/build-docs.yml/badge.svg?branch=main)](https://sgeist-ionos.github.io/R-zlib/)

[![Build tarball artifact on release](https://github.com/sgeist-ionos/R-zlib/actions/workflows/build.yml/badge.svg)](https://github.com/sgeist-ionos/R-zlib/actions/workflows/build.yml)
## Description

The `zlib` package for R aims to offer an R-based equivalent of Python's built-in `zlib` module for data compression and decompression. This package provides a suite of functions for working with zlib compression, including utilities for compressing and decompressing data streams, manipulating compressed files, and working with gzip, zlib, and deflate formats.

## Usage

This example demonstrates how to use the `zlib` Rcpp module for chunked compression and decompression. We will take a string, write it to a temporary file, and then read it back into a raw vector. Then we will compress and decompress the data using the `zlib` Rcpp module.

### Installation

To install the `zlib` package, you can use the following command:

```R
install.packages("zlib")  # Uncomment this line if zlib is hosted on CRAN or a similar repo
```

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
sudo apt-get update
sudo apt-get install cmake ninja-build r-base libblas-dev liblapack-dev build-essential
```

### Installing Dependencies on Red Hat

```bash
sudo yum update
sudo yum install cmake ninja-build R libblas-devel liblapack-devel gcc-c++
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

---

Feel free to modify this template to better suit the specifics of your project.
