@{
    'yt-dlp' = @{
        Url = 'https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe'
        Executable = 'yt-dlp.exe'
        Provider = 'github-allowlisted-https'
        PackageType = 'executable'
    }
    'deno' = @{
        Url = 'https://github.com/denoland/deno/releases/latest/download/deno-x86_64-pc-windows-msvc.zip'
        Executable = 'deno.exe'
        Provider = 'github-allowlisted-https'
        PackageType = 'zip'
        InnerExecutable = 'deno.exe'
    }
}
