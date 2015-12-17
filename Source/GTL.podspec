Pod::Spec.new do |s|

  s.name         = "GTL"
  s.version      = "2.0.9"
  s.summary      = "Google APIs Client Library for Objective-C"

  s.description  = <<-DESC
                  Written by Google, this library is a flexible and efficient Objective-C framework for accessing JSON APIs.

                  This is the recommended library for accessing JSON-based Google APIs for iOS and Mac OS X applications. The library is compatible with applications built for iOS 3 and later, and Mac OS X 10.5 and later.                   
                      DESC

  s.homepage     = "https://github.com/Skogetroll/google-api-objectivec-client.git"

  s.license      = { :type => "Apache License, Version 2.0", :file => "Source/LICENSE.txt" }

  s.author       = { "grobbins" => "grobbins@google.com" }

  s.source       = { :git => "https://github.com/Skogetroll/google-api-objectivec-client.git", :tag => "2.0.9" }

  s.default_subspec = "Core", "OAuth2", "YouTube", "Plus"
  
  s.subspec "HTTPFetcher" do |hp|
    hp.source_files = "Source/HTTPFetcher/*.{h,m}"

    hp.header_mappings_dir = "HTTPFetcher"
    hp.header_dir = "HTTPFetcher"
  end

  s.subspec "OAuth2" do |oa|
    oa.dependency "GTL/HTTPFetcher"

    oa.source_files     = "Source/OAuth2/*.{h,m}"
    oa.ios.source_files = "Source/OAuth2/Touch/*.{h,m}"
    oa.ios.resources    = "Source/OAuth2/Touch/*.xib"
    oa.osx.source_files = "Source/OAuth2/Mac/*.{h,m}"
    oa.osx.resources    = "Source/OAuth2/Mac/*.xib"

    oa.header_mappings_dir = "OAuth2"
    oa.header_dir = "OAuth2"
  end

  s.subspec "Core" do |core|
    core.dependency "GTL/HTTPFetcher"

    core.source_files = "Source/Objects/*.{h,m}", "Source/Utilities/*.{h,m}", "Source/GTLDefines.h"
  end

  s.subspec "YouTube" do |yt|
    yt.dependency "GTL/Core"
    
    yt.exclude_files = "Source/Services/YouTube/**/*_Sources.m"
    yt.source_files  = "Source/Services/YouTube/**/*.{h,m}"

    yt.header_mappings_dir = "YouTube"
    yt.header_dir = "YouTube"
  end

  s.subspec "Plus" do |pl|
    pl.dependency "GTL/Core"
  
    pl.exclude_files = "Source/Services/Plus/**/*_Sources.m"
    pl.source_files  = "Source/Services/Plus/**/*.{h,m}"

    pl.header_mappings_dir = "Plus"
    pl.header_dir = "Plus"
  end
  
  s.platform = :osx, "10.7"
  s.platform = :ios, "5.0"

  s.requires_arc = false

end
