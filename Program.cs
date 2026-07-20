using Microsoft.EntityFrameworkCore;
using System.Linq;

var builder = WebApplication.CreateBuilder(args);
builder.Services.AddDbContext<AlertDbContext>();
builder.Services.AddRazorPages();
var app = builder.Build();
app.MapRazorPages();

// insert into db
app.MapPost("/alert", async (AlertEvent dto, AlertDbContext db) =>
{
    db.Alerts.Add(dto);
    await db.SaveChangesAsync();
    Console.WriteLine($"Saved alert: {dto.Message} | Sound: {dto.SoundValue} |{dto.Timestamp}");
    return Results.Ok();
});

// read db
app.MapGet("/alerts",  async (AlertDbContext db) => 
{
    var alerts = await db.Alerts.ToListAsync();
    return Results.Ok(alerts);
});

app.Run("http://0.0.0.0:5238");


