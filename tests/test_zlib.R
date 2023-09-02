library(testthat)
library(zlib)

test_that("Chunked compression and decompression cycle retains data", {

  # Create a temporary file
  temp_file <- tempfile(fileext = ".txt")

  # Generate example data and write to the temp file
  example_data <- "This is an example string. It contains more than just 'hello, world!'"
  writeBin(charToRaw(example_data), temp_file)

  # Read data from the temp file into a raw vector
  file_con <- file(temp_file, "rb")
  raw_data <- readBin(file_con, "raw", file.info(temp_file)$size)
  close(file_con)

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

  # Convert decompressed raw vector back to string
  decompressed_str <- rawToChar(decompressed_data)

  # Check if decompressed string matches original string
  expect_equal(decompressed_str, example_data)
})
