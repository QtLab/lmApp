
This a tool that either:

  1. Extracts an independent non-base layer from a multi-layer bitstream, converts it to a base-layer bistream and writes it to a file. The extraction processs is defined in chapter F.10.2 of the HEVC version 2 specification.

or

  2. Extracts an additional layer set sub-bitstream from multi-layer bitstream and writes it to a file. The extraction processs is defined in chapter F.10.3 of the HEVC version 2 specification.


The tool is invoked as follows:

  ExtractAddLS <infile> <outfile> <Max temporal ID> <layer IDs of the extracted layers>

The process that is invoked is decided based on the number of IDs that are given in the layer ID list.  If only one layer ID is given, independent non-base layer rewriting process is invoked. If more than one layer ID is given, sub-bitstream extraction for additional layer sets is invoked.

The result of independent non-base layer rewriting process can be decoded with an HEVC/H.265 v1 compliant decoder as long as the extracted independent layer conform with v1 specification text. The tool removes VPS from the output bitstream during rewriting so an HEVC/H.265 decoder should not expect it to be present.




