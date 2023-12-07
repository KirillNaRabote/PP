using System;
using System.Diagnostics;
using System.Net.Http;
using System.Runtime.InteropServices.JavaScript;
using System.Text.Json.Nodes;
using System.Threading.Tasks;

class Program
{
    static async Task Main(string[] args)
    {
        static async Task DownloadImage(string apiUrl)
        {
            
            try
            {
                HttpClient client = new HttpClient();
                HttpResponseMessage response = await client.GetAsync(apiUrl);
                if (response.IsSuccessStatusCode)
                {
                    string responseContent = await response.Content.ReadAsStringAsync();

                    string imageUrl = JsonObject.Parse(responseContent)["message"].ToString();

                    Console.WriteLine($"Starting download from url: {imageUrl}");

                }
                else
                {
                    Console.WriteLine($"Failed to load the image, status code: {response.StatusCode}");

                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error receiving the response. {ex.Message}");
            }
        }

        const string apiUrl = "https://dog.ceo/api/breeds/image/random";
        const int numberOfImages = 10;

        // Асинхронная загрузка
        Console.WriteLine($"Async execution started.");
        var asyncWatch = Stopwatch.StartNew();
        List<Task> downloadTasks = new List<Task>();
        for (int i = 0; i < numberOfImages; i++)
        {
            downloadTasks.Add(DownloadImage(apiUrl));
        }
        await Task.WhenAll(downloadTasks);
        asyncWatch.Stop();
        Console.WriteLine($"Async execution time: {asyncWatch.ElapsedMilliseconds} ms");

        // Засекаем время начала выполнения синхронной загрузки
        Console.WriteLine($"Sync execution started.");
        var syncWatch = Stopwatch.StartNew();
        for (int i = 0; i < numberOfImages; i++)
        {
            await DownloadImage(apiUrl);
        }
        syncWatch.Stop();
        Console.WriteLine($"Sync Execution Time: {syncWatch.ElapsedMilliseconds} ms");
    }
}
