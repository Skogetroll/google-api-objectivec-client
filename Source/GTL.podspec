Pod::Spec.new do |s|

  s.name         = "GTL"
  s.version      = "2.0.0"
  s.summary      = "Google APIs Client Library for Objective-C"

  s.description  = <<-DESC
                  Written by Google, this library is a flexible and efficient Objective-C framework for accessing JSON APIs.

                  This is the recommended library for accessing JSON-based Google APIs for iOS and Mac OS X applications. The library is compatible with applications built for iOS 3 and later, and Mac OS X 10.5 and later.                   
                      DESC

  s.homepage     = "https://code.google.com/p/google-api-objectivec-client/"

  s.license      = { :type => "Apache License, Version 2.0", :file => "LICENSE.txt" }

  s.author       = { "grobbins" => "grobbins@google.com" }

  s.source       = { :git => "https://github.com/Skogetroll/google-api-objectivec-client.git", :tag => "2.0.0" }

  s.source_files  = "Source/GTLDefines.h", "Source/**/*.{h,m}"
  s.exclude_files = "Source/GTLNetworking_Sources.*", "Source/GTLCommon_Sources.*", "Source/Tools/*.*", "Source/Tools/**/*.*", "Source/Tests/*.*", "Source/Tests/**/*.*", "Source/**/Tests/*.*", "Source/Services/**/**/*.*"

  #s.platform = :osx
  s.platform = :ios

  # s.framework  = "SomeFramework"
  # s.frameworks = "SomeFramework", "AnotherFramework"

  # s.library   = "iconv"
  # s.libraries = "iconv", "xml2"


  s.requires_arc = false

end
