# Spark(less)
Qt-based command line program that automatically updates a sparkle appcast.xml file.This is still a work in progress in order to facilitate usage for other developer’s usecase.

----

### Usage

`sparkless [appcastFile] [remoteURL] [version] [buildNumber]`
A mac bundle and/or windows bundle must be added with their respective signatures

Valid usage examples

Mac
`sparkless appcast.xml "https://s3-us-west-2.amazonaws.com/bucket/releases" "1.0.0" 1 \
           -mF app-1.0.0-1.dmg -mS "SIGNATURE"`

Windows
`sparkless appcast.xml "https://s3-us-west-2.amazonaws.com/bucket/releases" "1.0.0" 1 \
           -wF appSetup-1.0.0-1.exe -wS "SIGNATURE"`

Mac & Windows
`sparkless appcast.xml "https://s3-us-west-2.amazonaws.com/bucket/releases" "1.0.0" 1 \
           -mF app-1.0.0-1.dmg -mS "SIGNATURE" \
           -wF appSetup-1.0.0-1.exe -wS "SIGNATURE"`
        
* If a bundle with the same version and OS is found sparkless will return an error.
This messages can be surpressed with -f and the matching enclosure values will be overwritten [THEY WILL NOT PLACED IN A NEW ITEM]


If a bundle has a matching version but no enclosure for the specified OS, the bundle will be placed in the existing item. This allows for easy maintanence of multple bundles.
`sparkless appcast.xml "https://s3-us-west-2.amazonaws.com/bucket/releases" "1.0.0" 1 \
           -mF app-1.0.0-1.dmg -mS "SIGNATURE"
sparkless appcast.xml "https://s3-us-west-2.amazonaws.com/bucket/releases" "1.0.0" 1 \
           -wF appSetup-1.0.0-1.exe -wS "SIGNATURE"`
           
In laymans terms - if both of these commands were run, the appcast would be generated successfully and both bundles would be listed under the same 'item' xml node.

----

### Usage Notes

* This is still a work in progress in order to facilitate usage for other developer’s usecase.
* The [RemoteURL] and [ReleaseNotesURL] attributes are both configured for a specific use case. Adjust as needed.
* Descriptions are temporarily disabled, and a flag to toggle description or release notes will be added.
