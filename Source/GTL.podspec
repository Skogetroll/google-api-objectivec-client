Pod::Spec.new do |s|

  s.name         = "GTL"
  s.version      = "2.0.2"
  s.summary      = "Google APIs Client Library for Objective-C"

  s.description  = <<-DESC
                  Written by Google, this library is a flexible and efficient Objective-C framework for accessing JSON APIs.

                  This is the recommended library for accessing JSON-based Google APIs for iOS and Mac OS X applications. The library is compatible with applications built for iOS 3 and later, and Mac OS X 10.5 and later.                   
                      DESC

  s.homepage     = "https://code.google.com/p/google-api-objectivec-client/"

  s.license      = { :type => "Apache License, Version 2.0", :file => "Source/LICENSE.txt" }

  s.author       = { "grobbins" => "grobbins@google.com" }

  s.source       = { :git => "https://github.com/Skogetroll/google-api-objectivec-client.git", :tag => "2.0.2" }

  # s.source_files  = "Source/GTLDefines.h", "Source/**/*.{h,m}"
  # s.exclude_files = "Sources/**/Mac/*.*", "Source/*_Sources.*", "Source/**/*_Sources.*", "Source/Tools/*.*", "Source/Tools/**/*.*", "Source/Tests/*.*", "Source/Tests/**/*.*", "Source/**/Tests/*.*", "Source/Services/**/**/*.*"

  s.default_subspec = "Core", "YouTube", "Plus"
  
  s.subspec "HTTPFetcher" do |hp|
    hp.source_files = "Source/HTTPFetcher/*.{h,m}"
  end

  s.subspec "Core" do |core|
    core.dependency "GTL/HTTPFetcher"

    core.source_files = "Source/Objects/*.{h,m}", "Source/Utilities/*.{h,m}", "Source/GTLDefines.h"
  end

  s.subspec "YouTube" do |yt|
    yt.dependency "GTL/Core"
    
    yt.exclude_files = "Source/Services/YouTube/**/*_Sources.m"
    yt.source_files  = "Source/Services/YouTube/**/*.{h,m}"
  end

  s.subspec "Plus" do |pl|
    pl.dependency "GTL/Core"
  
    pl.exclude_files = "Source/Services/Plus/**/*_Sources.m"
    pl.source_files  = "Source/Services/Plus/**/*.{h,m}"
  end
  
  s.platform = :osx, "10.7"
  s.platform = :ios, "5.0"

  s.frameworks = "Foundation", "UIKit"

  s.requires_arc = false

end
