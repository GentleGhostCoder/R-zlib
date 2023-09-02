# Function to generate index.html
generate_index_html <- function(html_dir = "html") {
  # Initialize HTML strings
  header <- '<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Your R Package Documentation</title>
  <style>
    #toc {
      float: left;
      width: 20%;
      margin-right: 10px;
    }
    #content {
      float: right;
      width: 75%;
    }
  </style>
</head>
<body>
  <div id="toc">
    <h1>Table of Contents</h1>
    <ul>
      <li><a href="README.html" target="content-frame">Introduction</a></li>'

  footer <- '</ul>
  </div>
  <div id="content">
    <iframe name="content-frame" style="width: 100%; height: 100vh;" src="README.html"></iframe>
  </div>
</body>
</html>'

  # List the html files in the specified directory
  html_files <- list.files(html_dir, full.names = TRUE, pattern = "\\.html$")

  # remove the index.html
  html_files <- html_files[grep("(index.html|README.html)", html_files, invert = TRUE)]

  # Create the list elements for the table of contents
  toc_list <- lapply(html_files, function(f) {
    filename <- basename(f)
    sprintf('<li><a href="%s" target="content-frame">%s</a></li>',
            filename, tools::file_path_sans_ext(filename))
  })

  # Combine all the HTML components
  full_html <- paste0(header, paste0(unlist(toc_list), collapse = "\n"), footer)

  # Write to index.html
  writeLines(full_html, paste0(html_dir, "/index.html"))
}

out_path <- "html"

# create html
dir.create(out_path)

# create docs from man
files <- list.files("man", full.names = TRUE);
lapply(files, function(f)
  tools::Rd2HTML(f, gsub("man", out_path, gsub(".Rd", ".html", f))));

# create main README
rmarkdown::render("README.md", output_file = paste0(out_path, "/README.html"))

# Generate the index.html
generate_index_html(out_path)


# Work in progress ...