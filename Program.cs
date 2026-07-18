using Microsoft.EntityFrameworkCore;

var builder = WebApplication.CreateBuilder(args);
builder.Services.AddDbContext<AlertDbContext>();
var app = builder.Build();

app.MapPost("/alert", async (AlertEvent evt, AlertDbContext db) =>
{
    db.Alerts.Add(evt);
    await db.SaveChangesAsync();
    Console.WriteLine($"Saved alert: {evt.Message} | Sound: {evt.SoundValue}");
    return Results.Ok();
});

app.Run("http://0.0.0.0:5238");

public class AlertEvent
{   
     public int Id {get; set;}
    public string Message{get; set;} = "";
    public int SoundValue{get; set;}
}

public class AlertDbContext : DbContext
{
    public DbSet<AlertEvent> Alerts {get; set;}

    protected override void OnConfiguring(DbContextOptionsBuilder options) => options.UseSqlite("Data Source=alerts.db");
} 
