#!/usr/bin/perl

# This script converts a human readable file for all the
# language 'Resources' for the Chameleon package installer
# in to a simple format ready for merging with pre-defined
# templates during compilation.
#
# The human readable file is hosted on Google Docs at:
# www.......
# The file should be saved from Google Docs by using:
# File -> Download As -> Text (current sheet)
#
# This saves a UTF-8, tabbed delimited text file which
# should be placed in the same folder as this script.
#

if ($#ARGV < 0) {
   print stderr "A destination file is needed\n";
} else {
   $destination_file=$ARGV[0];
}

sub set_indent { 
# Set indent to 4 spaces X the number of fields deep currently at.
# $_[0] = Headed field number (not including blanks) currently being processed.
    $new_indent="    ";
    for ($l = 0; $l < $_[0]; $l++) {
        $new_indent=$new_indent."    ";
    }
    return $new_indent;
}

sub adjust_indent { 
# Pull back indent if Headed fields previous to current one haven't yet been populated.
# $_[0] = Current indent being used which would have come from function set_indent().
# $_[1] = Headed field number (not including blanks) currently being processed.
    $new_indent= $_[0];
    for ($p = 0; $p < $_[1]; $p++) {
    if ( $fieldsWithHeadersSet[$p] eq 0 ) {
            $new_indent = substr($new_indent, 0, -4);
        }
    }
    return $new_indent;
}

sub calculateIndent {
# Find number of indents required by matching name of fieldsWithoutLanguage to position in fieldsWithHeaders.
    for($i = 0; $i < scalar(@fieldsWithHeaders); $i++) {
        if ( $fieldsWithHeaders[$i] eq $fieldsWithoutLanguage[$loop] ) {
            $indent=set_indent($i);
            last;
        }
    }
    $indent=adjust_indent($indent,$i);  # Are there any empty 'headed' fields preceding this one? if so, adjust.
}

#--------------------------------------------------------------------------
$sourceFileToRead="Package%20Installer%20Resource%20Text%20-%20Sheet1.tsv";
#--------------------------------------------------------------------------

open (FILE, "$sourceFileToRead");
open (OUTFILE, ">$destination_file");

print OUTFILE "\n";                                                                     # Output blank line. 
                                                                                        # This is needed so after converting to rtf using textutil in later bash script
                                                                                        # the rtf description header appears on it's own line. For example: \f0\fs24 \cf0..
while (<FILE>) {
    $indent = "    ";
    chomp;
    @fieldsReadIn = split("\t");
    $startFirstField=substr($fieldsReadIn[0],0,1);                                      # First charatcer of first field
	$startSecondField=substr($fieldsReadIn[1],0,1);                                     # First charatcer of second field

    if ($startFirstField ne "#" ) {                                                     # Ignore lines beginning with a hash
        if ($startFirstField ne "") {                                                   # Check first field for language identifier.
            print OUTFILE "language: $fieldsReadIn[0]\n";                               # Output first field - LANGUAGE.
        }
        $headedFieldCount=0;
        if ($startSecondField ne "") {                                                  # Check second field for language identifier.
            print OUTFILE "    file: $fieldsReadIn[1]\n";                               # Output second field - FILE.       
            @fieldsWithHeaders = ();                                                    # Clear array which stores only field names with titles and NOT blanks.
            for($i = 1; $i <= scalar(@fieldsReadIn); $i++) {                            # Loop through each field - ignoring first field [0].
                @fieldsWithoutLanguage[$i]=$fieldsReadIn[$i];                           # copy each field in to 'fieldsWithoutLanguage' array.
                if ( $fieldsWithoutLanguage[$i] ne "" ) {                               # Check for NON-Blank fields headers
                    push(@fieldsWithHeaders, $fieldsWithoutLanguage[$i]);               # Store in to 'fieldsWithHeaders' array.
                }
            }
        }
       
        for ($loop = 1; $loop < scalar(@fieldsReadIn); $loop++) {
            if ( $fieldsReadIn[$loop] ne "" && $fieldsWithoutLanguage[$loop] ne "" ) {
                if ( $fieldsReadIn[$loop] ne $fieldsWithoutLanguage[$loop]) { 
	                $headedFieldCount++;
                    $indent=calculateIndent;
                    print OUTFILE $indent."$fieldsWithoutLanguage[$loop]: $fieldsReadIn[$loop]\n";
                    $fieldsWithHeadersSet[$headedFieldCount]=1;                          # Set value in this array to mark that we've written to it.
                }
            }    
        }
    }
}
close (FILE);
close (OUTFILE);
exit;