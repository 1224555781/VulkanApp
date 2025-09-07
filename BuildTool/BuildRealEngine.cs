using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using BuildTools;   

class BuildRealEngine
{
    static void Main(string[] args)
    {
        Console.WriteLine("Starting BuildRealEngine...");

        // Load the build configuration
        BuildConfig config = BuildConfig.Load();
        config.AddCompilerFlag("/std:c++20");
        config.AddCompilerFlag("/MD");
        // Process command line arguments for compiler flags
        ProcessCommandLineArgs(args, config);

        string glfwFolderPath = Path.Combine("..", "RealEngine", "GLFW");
        string glfwDownloadUrl = "https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip";
        string glfwZipPath = Path.Combine("..", "RealEngine", "glfw-3.4.zip");

        Console.WriteLine("Checking GLFW folder...");
        EnsureGLFW(glfwFolderPath, glfwDownloadUrl, glfwZipPath);

        string ninjaPath = "ninja";
        
        // Get the parallel jobs count from config
        int processorCount = config.ParallelJobs;
        
        // Generate the CMake command using the configuration
        string cmakeCommand = $"cmake {config.GenerateCMakeArguments()}";

        Console.WriteLine("Checking if Ninja is installed...");
        if (!IsToolAvailable(ninjaPath))
        {
            Console.WriteLine("Ninja not found. Installing using winget...");
            InstallNinja();
        }
        else
        {
            Console.WriteLine("Ninja is already installed.");
        }

        // Display the current configuration before running
        config.DisplayConfig();

        Console.WriteLine("Running the cmake command...");
        RunCommand(cmakeCommand);

        // Save the configuration for future use
        config.Save();

        Console.WriteLine("Build process completed.");
    }

    static void ProcessCommandLineArgs(string[] args, BuildConfig config)
    {
        for (int i = 0; i < args.Length; i++)
        {
            string arg = args[i];
            
            // Handle compiler flags (starting with -)
            if (arg.StartsWith("-"))
            {
                config.AddCompilerFlag(arg);
            }
            // Handle custom parameters (in form of key=value)
            else if (arg.Contains("="))
            {
                string[] parts = arg.Split('=', 2);
                if (parts.Length == 2)
                {
                    config.AddParameter(parts[0], parts[1]);
                }
            }
        }
    }

    static void EnsureGLFW(string folderPath, string downloadUrl, string zipPath)
    {
        if (!Directory.Exists(folderPath) || Directory.GetFiles(folderPath, "CMakeLists.txt", SearchOption.TopDirectoryOnly).Length == 0)
        {
            Console.WriteLine("GLFW folder is empty or missing Makefile. Downloading GLFW 3.4...");
            DownloadAndExtractGLFW(downloadUrl, zipPath, folderPath);
        }
        else
        {
            Console.WriteLine("GLFW folder is valid.");
        }
    }

    static void DownloadAndExtractGLFW(string url, string zipPath, string extractPath)
    {
        try
        {
            Console.WriteLine($"Downloading GLFW from {url}...");
            using (WebClient client = new WebClient())
            {
                client.DownloadFile(url, zipPath);
            }
            Console.WriteLine("Download completed. Extracting GLFW...");

            string tempExtractPath = Path.Combine(Path.GetTempPath(), "GLFW_Temp");
            if (Directory.Exists(tempExtractPath))
            {   
                Directory.Delete(tempExtractPath, true);
            }

            System.IO.Compression.ZipFile.ExtractToDirectory(zipPath, tempExtractPath);
            Console.WriteLine("Extraction completed.");

            // Detect the nested folder (e.g., glfw-3.4) and adjust the source path
            string nestedFolderPath = Directory.GetDirectories(tempExtractPath, "*", SearchOption.TopDirectoryOnly)[0];

            Console.WriteLine($"Copying extracted files from {nestedFolderPath} to {extractPath}...");
            if (!Directory.Exists(extractPath))
            {
                Directory.CreateDirectory(extractPath);
            }

            foreach (string dirPath in Directory.GetDirectories(nestedFolderPath, "*", SearchOption.AllDirectories))
            {
                Directory.CreateDirectory(dirPath.Replace(nestedFolderPath, extractPath));
            }

            foreach (string filePath in Directory.GetFiles(nestedFolderPath, "*.*", SearchOption.AllDirectories))
            {
                File.Copy(filePath, filePath.Replace(nestedFolderPath, extractPath), true);
            }

            Console.WriteLine("Files copied successfully.");

            Directory.Delete(tempExtractPath, true);
            Console.WriteLine("Temporary extraction folder deleted.");

            File.Delete(zipPath);
            Console.WriteLine("Temporary zip file deleted.");
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"Error downloading or extracting GLFW: {ex.Message}");
        }
    }

    static bool IsToolAvailable(string toolName)
    {
        Console.WriteLine($"Checking availability of tool: {toolName}...");
        try
        {
            Process process = new Process();
            process.StartInfo.FileName = "where";
            process.StartInfo.Arguments = toolName;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.CreateNoWindow = true;
            process.Start();

            string output = process.StandardOutput.ReadToEnd();
            process.WaitForExit();

            return !string.IsNullOrWhiteSpace(output);
        }
        catch
        {
            return false;
        }
    }

    static void InstallNinja()
    {
        Console.WriteLine("Installing Ninja using winget...");
        RunCommand("winget install -e --id Ninja-build.Ninja");
        Console.WriteLine("Ninja installation completed.");
    }

    static void RunCommand(string command)
    {
        Console.WriteLine($"Executing command: {command}");
        try
        {
            string vsCommandPrompt = FindVSCommandPrompt();
            Process process = new Process();

            if (!string.IsNullOrEmpty(vsCommandPrompt))
            {
                Console.WriteLine("Using Visual Studio x64 Native Command Prompt...");
                process.StartInfo.FileName = vsCommandPrompt;
                process.StartInfo.Arguments = $"/c {command}";
            }
            else
            {
                Console.WriteLine("Using default command prompt...");
                process.StartInfo.FileName = "cmd.exe";
                process.StartInfo.Arguments = $"/c {command}";
            }

            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.CreateNoWindow = true;

            process.OutputDataReceived += (sender, e) => { if (e.Data != null) Console.WriteLine(e.Data); };
            process.ErrorDataReceived += (sender, e) => { if (e.Data != null) Console.Error.WriteLine(e.Data); };

            process.Start();
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            process.WaitForExit();

            if (process.ExitCode != 0)
            {
                throw new Exception($"Command failed with exit code {process.ExitCode}");
            }

            Console.WriteLine("Command executed successfully.");
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"Error: {ex.Message}");
        }
    }

    static string FindVSCommandPrompt()
    {
        Console.WriteLine("Searching for Visual Studio x64 Native Command Prompt...");
        try
        {
            string programFiles = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86);
            string vsPath = Path.Combine(programFiles, "Microsoft Visual Studio");
            if (Directory.Exists(vsPath))
            {
                foreach (var versionDir in Directory.GetDirectories(vsPath))
                {
                    string toolsPath = Path.Combine(versionDir, "BuildTools", "VC", "Auxiliary", "Build", "vcvars64.bat");
                    if (File.Exists(toolsPath))
                    {
                        return toolsPath;
                    }
                }
            }
        }
        catch
        {
            // Ignore errors and return null
        }
        Console.WriteLine("Visual Studio x64 Native Command Prompt search completed.");
        return null;
    }
}
