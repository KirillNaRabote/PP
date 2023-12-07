using System.IO;

class Program
{
    static async Task Main(string[] args)
    {
        Console.WriteLine("Path to file: ");
        string fileName = Console.ReadLine();

        if (!File.Exists(fileName))
        {
            Console.WriteLine("The file does not exist");
            return;
        }

        Console.WriteLine("Symbols to delete: ");

        string charsToDelete = Console.ReadLine();
        string[] result = { };

        string[] text = await File.ReadAllLinesAsync(fileName);

        string? line;

        for (int i = 0; i < text.Length; i++)
        {
            line = text[i];
            foreach (var c in charsToDelete)
            {
                line = line.Replace(c.ToString(), string.Empty);
            }
            Array.Resize(ref result, result.Length + 1);
            result[result.Length - 1] = line;
        }
        Console.Write(result);
        await File.WriteAllLinesAsync(fileName, result);
    }
}