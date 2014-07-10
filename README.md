In an effort to improve processing, we've benchmarked 4 different open-source PDF renderers.  Graphics Magick (DocumentCloud's current renderer), MuPDF, PDFIUM, and Poppler.

We measured execution time, memory consumption, and size of the resulting image files.   Testing was performed using the `time` command with the `-l` flag on OSX.  That flag compels time to output the contents of the rusage structure, which includes the RSS (resident set size) and execution time.

Various PDF's where choosen for evaluation, one image heavy PDF that is currently slow to extract, and several other "normal" 2-4 pages. None of our collection of problematic pdfs were included. Testing was performed outside the scope of the benchmark on a PDF that currently locks up GM and they all processed it successfully.

Results
-----
* Times are number of seconds elapsed.
* The results are shown as Gif / PNG formats

| |Time|Memory|File Size|
| ------------- | -----------:| -------------:| -----------:|
|MUPDF|15.52 / 8.77|55.95MB / 55.56MB|15M / 37M|
|PDFIUM|11.68 / 16.69|58.51MB / 57.67MB|17M / 37M|
|GM|202.92 / 183.54|991.47MB / 992.05MB|17M / 132M|
|POPPLER|71.45 / 69.26|28.05MB / 28.06MB|44M / 44M|

#### Notes
---
PDFIUM and MuPDF are neck and neck in the results.  MuPDF is faster at PNG generation, while slightly slower at GIFs. Memory consumption and file ssize are about equal between the two.

Poppler is a bit slower, but that is most likely the result of running it via the pdftocairo utility vs a custom workflow.  GraphicsMagick performance is very bad, **almost 20 times slower**.

Memory usage of Graphics Magic was also very surprising.  It used almost a GB of memory, while the other tools stayed around 50MB.

GraphicsMagick also used an inordinate amount of disk space for PNG generation.  This could most likely be fixed by fine-tuning the parameters for the command.

Running the benchmarks
--------------

The benchmarks may be generated via the `benchmark` script.  The script runs the mupdf, pdfium, gm and poppler commands that are located in the "bin" directory.

The source code for the mupdf and pdfium binaries, and a Makefile to build them is also located in the "bin" directory. The binaries for mupdf and pdfium are staticly linked to the appropriate libraries, and should be easily runnable on a OSX system (*this is untested however*).  They do depend on the freeimage library, which can be installed via `brew install freeimage`

GraphicsMagick and Poppler must also be installed with the brew packages "imagemagick" and "poppler" packages.


Issues with the tests
--------------

Despite our best efforts, there remain portions of the test that are not a fair comparison.

####Comamnd line vs custom application
GraphicsMagic and poppler conversion are performed using their built-in extraction commands (convert & pdfpdftocairo respectively).
They each require a separate invocation for each extraction size, therefore they must startup 4 times per PDF.

The test programs for MuPDF and PDFIUM are more efficient in their usage, and are only started once, loop through each page and then resize each page 4 times.  Some of their respective slowness can no doubt be linked to this.

####Poppler

Poppler doesn't support extracting to GIF images.  Currently the benchmark performs PNG extraction for both runs.  An enhancement would be to modify the source to perform GIF extraction.  Such a modification should not be difficult since Cairo does support GIF images already.
