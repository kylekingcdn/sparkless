# Spark(less)
Spark(less) is a Qt-based command line program that automatically updates a sparkle appcast.xml file. Upon completion Sparkless will facilitate usage for other developers' needs.

----

### Usage

`sparkless [appcastFile] [remoteURL] [version] [buildNumber]`

For proper usage Mac or Windows bundles are required along with their respective signatures.

#### Valid usage examples

###### Mac

```c++
sparkless ./appcast.xml "https://s3-us-west-2.amazonaws.com/bucket/releases" "1.0.0" 1 \
          -mF ./app-1.0.0-1.dmg -mS "SIGNATURE"
```

###### Windows

```c++
sparkless ./appcast.xml "https://s3-us-west-2.amazonaws.com/bucket/releases" "1.0.0" 1 \
          -wF ./appSetup-1.0.0-1.exe -wS "SIGNATURE"
```

###### Mac & Windows Simultaneously

```c++
sparkless ./appcast.xml "https://s3-us-west-2.amazonaws.com/bucket/releases" "1.0.0" 1 \
          -mF ./app-1.0.0-1.dmg -mS "SIGNATURE" \
          -wF ./appSetup-1.0.0-1.exe -wS "SIGNATURE"
```

----

### Usage Notes

* The [RemoteURL] and [ReleaseNotesURL] attributes are both configured for a specific use case and can be adjusted as required.
* Descriptions are temporarily disabled, and a flag to toggle description or release notes will be added. If a bundle with the same version and OS is found sparkless will return an error.
This message can be supressed with -f and the matching enclosure values will be overwritten but will not be placed in a new item.

* If a bundle has a matching version but no enclosure for the specified OS, the bundle will be placed in the existing item. This allows for easy maintenance of multiple bundles.

```c++
sparkless ./appcast.xml "https://s3-us-west-2.amazonaws.com/bucket/releases" "1.0.0" 1 \
          -mF ./app-1.0.0-1.dmg -mS "SIGNATURE"
sparkless ./appcast.xml "https://s3-us-west-2.amazonaws.com/bucket/releases" "1.0.0" 1 \
          -wF ./appSetup-1.0.0-1.exe -wS "SIGNATURE"
```
           
* If the above commands are run, the appcast will generate successfully and both bundles would be listed under the same 'item' xml node.
