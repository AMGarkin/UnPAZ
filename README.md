# Garkin's UnPAZ v1.2
Tool for extracting Black Desert Online archives.  

### UnPAZ \<input file\> \<commands\>  

*\<input file\>*:  name of .meta or .paz file (default: pad00000.meta)  
*\<commands\>*:  
&nbsp;&nbsp;*-f \<mask\>*:  Filter, this argument must be followed by mask. Mask supports wildcards * and ?.  
&nbsp;&nbsp;*-o \<path\>*:  Output folder, this argument must be followed by path.  
&nbsp;&nbsp;*-h*:  Print this help text  
&nbsp;&nbsp;*-l*:  List file names without extracting them  
&nbsp;&nbsp;*-n*:  No folder structure, extract files directly to output folder.  
&nbsp;&nbsp;*-y*:  Yes to all questions (creating folders, overwritting files).  
&nbsp;&nbsp;*-q*:  Quiet (limit printed messages to file names)  
  
### Examples:  
&nbsp;&nbsp;UnPAZ pad00001.paz -f *.luac   
&nbsp;&nbsp;UnPAZ pad00000.meta -y -f *languagedata_??.txt -o Extracted  
&nbsp;&nbsp;UnPAZ D:\Games\BlackDesert\live\Paz\pad00000.meta -l  
