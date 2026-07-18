var builder = WebApplication.CreateBuilder(args);
var app = builder.Build();

app.MapPost("/alert", (AlertEvent evt) =>
{
    Console.WriteLine($"Received alert: {evt.Message} | Sound: {evt.SoundValue}");
    return Results.Ok();
});

app.Run("http://0.0.0.0:5238");

record AlertEvent(string Message, int SoundValue);