using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

namespace BuildTools
{
    public class BuildConfig
    {
        // Basic build parameters
        public string Generator { get; set; } = "Ninja";
        public string Compiler { get; set; } = "CL";
        public string BuildType { get; set; } = "Debug";
        public string CMakePolicyVersion { get; set; } = "3.5";
        public int ParallelJobs { get; set; } = Math.Max(1, Environment.ProcessorCount - 2);
        
        // Compiler flags
        public List<string> CompilerFlags { get; set; } = new List<string>();
        
        // Additional custom parameters
        public Dictionary<string, string> CustomParameters { get; set; } = new Dictionary<string, string>();
        
        // Path to configuration file
        private static readonly string ConfigFilePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "build_config.json");

        // Constructor
        public BuildConfig()
        {
        }

        // Load configuration from file
        public static BuildConfig Load()
        {
            if (File.Exists(ConfigFilePath))
            {
                try
                {
                    string jsonString = File.ReadAllText(ConfigFilePath);
                    BuildConfig config = JsonSerializer.Deserialize<BuildConfig>(jsonString);
                    Console.WriteLine("Build configuration loaded successfully.");
                    return config;
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error loading configuration: {ex.Message}");
                }
            }

            Console.WriteLine("Using default build configuration.");
            return new BuildConfig();
        }

        // Save configuration to file
        public void Save()
        {
            try
            {
                var options = new JsonSerializerOptions { WriteIndented = true };
                string jsonString = JsonSerializer.Serialize(this, options);
                File.WriteAllText(ConfigFilePath, jsonString);
                Console.WriteLine("Build configuration saved successfully.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error saving configuration: {ex.Message}");
            }
        }

        // Generate CMake command line arguments based on configuration
        public string GenerateCMakeArguments(string buildDir = "./bin")
        {
            List<string> args = new List<string>
            {
                $"-B {buildDir}",
                $"-G \"{Generator}\"",
                $"-DCMAKE_CXX_COMPILER={Compiler}",
                $"-DCMAKE_BUILD_TYPE={BuildType}",
                $"-DCMAKE_POLICY_VERSION_MINIMUM={CMakePolicyVersion}",
                $"-DCMAKE_BUILD_PARALLEL_LEVEL={ParallelJobs}"
            };

            // Add compiler flags if any
            if (CompilerFlags.Count > 0)
            {
                string flagsString = string.Join(" ", CompilerFlags);
                args.Add($"-DCMAKE_CXX_FLAGS=\"{flagsString}\"");
            }

            // Add any custom parameters
            foreach (var param in CustomParameters)
            {
                args.Add($"-D{param.Key}={param.Value}");
            }

            return string.Join(" ", args);
        }

        // Helper method to add a compiler flag
        public void AddCompilerFlag(string flag)
        {
            if (!CompilerFlags.Contains(flag))
            {
            
                CompilerFlags.Add(flag);
                Console.WriteLine($"Added compiler flag: {flag}");
            }
        }

        // Helper method to remove a compiler flag
        public bool RemoveCompilerFlag(string flag)
        {
            return CompilerFlags.Remove(flag);
        }

        // Helper method to add a custom parameter
        public void AddParameter(string key, string value)
        {
            CustomParameters[key] = value;
        }

        // Helper method to remove a custom parameter
        public bool RemoveParameter(string key)
        {
            return CustomParameters.Remove(key);
        }

        // Display the current configuration
        public void DisplayConfig()
        {
            Console.WriteLine("Current Build Configuration:");
            Console.WriteLine($"Generator: {Generator}");
            Console.WriteLine($"Compiler: {Compiler}");
            Console.WriteLine($"Build Type: {BuildType}");
            Console.WriteLine($"CMake Policy Version: {CMakePolicyVersion}");
            Console.WriteLine($"Parallel Jobs: {ParallelJobs}");
            
            if (CompilerFlags.Count > 0)
            {
                Console.WriteLine("Compiler Flags:");
                foreach (var flag in CompilerFlags)
                {
                    Console.WriteLine($"  {flag}");
                }
            }
            
            if (CustomParameters.Count > 0)
            {
                Console.WriteLine("Custom Parameters:");
                foreach (var param in CustomParameters)
                {
                    Console.WriteLine($"  {param.Key}: {param.Value}");
                }
            }
        }
    }
}
