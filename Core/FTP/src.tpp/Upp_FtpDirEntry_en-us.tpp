topic "Ftp::DirEntry";
[0 $$1,0#96390100711032703541132217272105:end]
[i448;a25;kKO9;2 $$2,0#37138531426314131252341829483380:class]
[l288;2 $$3,3#27521748481378242620020725143825:desc]
[H6;0 $$4,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$5,0#37138531426314131252341829483370:item]
[l288;a4;*@5;1 $$6,6#70004532496200323422659154056402:requirement]
[ $$0,0#00000000000000000000000000000000:Default]
[{_} 
[ {{10000@(113.42.0) [s0;%% [*@7;4 Ftp`::DirEntry]]}}&]
[s1;%% &]
[s2;:Ftp`:`:DirEntry`:`:class: [@(0.0.255)3 class][3 _][*3 DirEntry][3 _:_][@(0.0.255)3 private][3 _
][*@3;3 Moveable][3 <][_^Ftp`:`:DirEntry^3 Ftp`::DirEntry][3 >_]&]
[s3;%% This nested class is intended to simplify the parsing of file 
system objects (files, directories, symbolic links) returned 
by the [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:class^ Ftp] class. 
It can handle both UNIX and DOS style directory listings.&]
[s1; &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:User`(const String`&`): [_^Ftp`:`:DirEntry^ DirEntry][@(0.0.255) `&
]_[* User]([@(0.0.255) const]_[_^String^ String][@(0.0.255) `&]_[*@3 u])&]
[s3;%% Sets user to [%-*@3 u]. If user is not set or is set to Null, 
public permissions can be queried.&]
[s1;%% &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:GetName`(`)const: [_^String^ String]_[* GetName]()_[@(0.0.255) cons
t]&]
[s3;%% Returns the name of the file system object.&]
[s1; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:GetRealName`(`)const: [_^Upp`:`:String^ String]_[* GetReal
Name]()_[@(0.0.255) const]&]
[s3;%% Returns the real path of the symbolic link, if available.&]
[s6;%% Only applicable to UNIX style directory listing.&]
[s1; &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:GetOwner`(`)const: [_^String^ String]_[* GetOwner]()_[@(0.0.255) co
nst]&]
[s3;%% Returns the owner of the file system object. &]
[s6;%% Only applicable to UNIX style directory listing.&]
[s1; &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:GetGroup`(`)const: [_^String^ String]_[* GetGroup]()_[@(0.0.255) co
nst]&]
[s3;%% Returns the group of the file system object. &]
[s6;%% Only applicable to UNIX style directory listing.&]
[s1;%% &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:GetSize`(`)const: [_^int64^ int64]_[* GetSize]()_[@(0.0.255) const]&]
[s3;%% Returns the size of the file system object.&]
[s1; &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:GetLastModified`(`)const: [_^Time^ Time]_[* GetLastModified]()_[@(0.0.255) c
onst]&]
[s3;%% Returns the last modification time of the file system object.&]
[s1; &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:GetEntry`(`)const: [_^String^ String]_[* GetEntry]()_[@(0.0.255) co
nst]&]
[s3;%% Returns the raw directory listing string.&]
[s1; &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:IsFile`(`)const: [@(0.0.255) bool]_[* IsFile]()_[@(0.0.255) const]&]
[s3;%% Returns true if the file system object is a file.&]
[s1; &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:IsDirectory`(`)const: [@(0.0.255) bool]_[* IsDirectory]()_[@(0.0.255) c
onst]&]
[s3;%% Returns true if the file system object a directory.&]
[s1; &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:IsSymLink`(`)const: [@(0.0.255) bool]_[* IsSymLink]()_[@(0.0.255) c
onst]&]
[s3;%% Returns true if the file system object is a symbolic link.&]
[s1; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:IsReadOnly`(`)const: [@(0.0.255) bool]_[* IsReadOnly]()_[@(0.0.255) c
onst]&]
[s3;%% Returns true if the file system object has read`-only access.&]
[s1; &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:IsReadable`(`)const: [@(0.0.255) bool]_[* IsReadable]()_[@(0.0.255) c
onst]&]
[s3;%% Returns true if the file system object is readable by the 
user. If user is not set, this method will return the public 
read permission. &]
[s6;%% Only applicable to UNIX style directory listing.&]
[s1; &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:IsWriteable`(`)const: [@(0.0.255) bool]_[* IsWriteable]()_[@(0.0.255) c
onst]&]
[s3;%% Returns true if  the file system object is writeable by the 
user. If user is not set, this method will return the public 
write permission. &]
[s6;%% Only applicable to UNIX style directory listing.&]
[s1; &]
[s4; &]
[s5;:Ftp`:`:DirEntry`:`:IsExecutable`(`)const: [@(0.0.255) bool]_[* IsExecutable]()_[@(0.0.255) c
onst]&]
[s3;%% Returns true if  file system object is executable by the user. 
If user is not set, this method will return the public execute 
permission. &]
[s6;%% Only applicable to UNIX style directory listing.&]
[s1; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:ToString`(`)const: [_^Upp`:`:String^ String]_[* ToString](
)_[@(0.0.255) const]&]
[s3;%% Returns the textual representation of the file system object`'s 
information.&]
[s1; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:ToXml`(`)const: [_^Upp`:`:String^ String]_[* ToXml]()_[@(0.0.255) c
onst]&]
[s3;%% Returns the information of the file system object as an xml 
string. Returned string will be a single xml tag, with the name 
of the file system object as its text:&]
[s3; &]
[ {{4133:5867<288;>544;l/26r/26t/14b/14@1-1 [s0;=%% [*2 ftp:direntry]]
::l/25r/25t/15b/15@2 [s0; ]
::l/26r/26t/14b/14@(178) [s0;=%% [2 Attributes]]
:: [s0;=%% [2 Possible Value(s)]]
::l/25r/25t/15b/15@2 [s0;=%% [*C2 style]]
:: [s0;%% [C2 dos, unix]]
:: [s0;=%% [*C2 type]]
:: [s0;%% [C2 file, directory, symlink, other]]
:: [s0;=%% [*C2 owner]]
:: [s0;%% [C2 A string representing the name of the owner.]]
:: [s0;=%% [*C2 group]]
:: [s0;%% [C2 A string representing the name of the group]]
:: [s0;=%% [*C2 realname]]
:: [s0;%% [C2 A string representation of the path pointed by the symbolic 
link.]]
:: [s0;=%% [*C2 size]]
:: [s0;%% [C2 A string representing an int64 value]]
:: [s0;=%% [*C2 modified]]
:: [s0;%% [C2 Last modification time in YYYY:MM`::DD HH:MM:SS format]]
:: [s0;=%% [*C2 permissions]]
:: [s0;%% [C2 rwxrwxrwx]]}}&]
[s3;%% &]
[s1; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:IsDosStyle`(`)const: [@(0.0.255) bool]_[* IsDosStyle]()_[@(0.0.255) c
onst]&]
[s3;%% Returns true if the directory entry is of DOS style.&]
[s1; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:IsUnixStyle`(`)const: [@(0.0.255) bool]_[* IsUnixStyle]()_
[@(0.0.255) const]&]
[s3;%% Returns true if the directory entry is of UNIX style.&]
[s1; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:GetStyle`(`)const: [_ DirEntry`::Style]_[* GetStyle]()_[@(0.0.255) c
onst]&]
[s3;%% Returns the directory listing style. Currently [^topic`:`/`/trunk`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:DirEntry`:`:Style`:`:UNIX^ U
NIX]  and [^topic`:`/`/trunk`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:DirEntry`:`:Style`:`:DOS^ D
OS] style directory listings are supported.&]
[s1;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:Style`:`:UNIX: [@(0.0.255) enum]_DirEntry`::Style`::[*@3 UNI
X]&]
[s3;%% Represents a UNIX style directory listing.&]
[s1; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:Style`:`:DOS: [@(0.0.255) enum]_DirEntry`::Style`::[*@3 DOS]&]
[s3;%% Represents a DOS style directory listing.&]
[s1;%% &]
[s0;%% &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s0; &]
[s5;:Ftp`:`:DirEntry`:`:DirEntry`(`): [* DirEntry]()&]
[s3;%% Default constructor.&]
[s1; &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 Ftp`::DirList]]}}&]
[s0; &]
[s5;:Ftp`:`:DirList`:`:typedef: [@(0.0.255) typedef]_[_^Vector^ Vector]<Ftp`::[_^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:DirEntry`:`:class^ D
irEntry]>_[* DirList]&]
[s3;%% Ftp`::DirList is a [^topic`:`/`/Core`/src`/Vector`$en`-us^ Vector] 
type container, containing [^topic`:`/`/FTP`/src`/FtpDirEntry`$en`-us`#Ftp`:`:DirEntry`:`:class^ F
tp`::DirEntry] elements.&]
[s1; &]
[s0;%% ]]