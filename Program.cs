using Microsoft.EntityFrameworkCore;

var builder = WebApplication.CreateBuilder(args);
builder.Services.AddDbContext<AlertDbContext>();
builder.Services.AddRazorPages();
var app = builder.Build();
app.MapRazorPages();

// insert into db
app.MapPost("/alert", async (AlertEventDtos dto, AlertDbContext db) =>
{
    var evt = new AlertEvent
    {
        Message = dto.Message,
        SoundValue = dto.SoundValue,
        Timestamp = DateTime.Now
    };

    db.Alerts.Add(evt);
    await db.SaveChangesAsync();
    Console.WriteLine($"Saved Alert;  {evt.Message} | Sound : {evt.SoundValue} | Timestamp: {evt.Timestamp}");
    return Results.Ok();
});

// read db
app.MapGet("/alerts",  async (AlertDbContext db) => 
{
    var alerts = await db.Alerts.ToListAsync();
    return Results.Ok(alerts);
});

app.Run("http://0.0.0.0:5238");


