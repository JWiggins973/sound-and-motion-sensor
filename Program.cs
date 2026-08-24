using Microsoft.EntityFrameworkCore;

var builder = WebApplication.CreateBuilder(args);
builder.Services.AddDbContext<AlertDbContext>();
builder.Services.AddSingleton<LiveStatusStore>();
builder.Services.AddRazorPages(options =>
{
    options.Conventions.AddPageRoute("/Live", "");
});
var app = builder.Build();
app.MapRazorPages();

using (var scope = app.Services.CreateScope())
{
    var db = scope.ServiceProvider.GetRequiredService<AlertDbContext>();
    db.Database.Migrate();
}

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

// update live status
app.MapPost("/update-live-status", (LiveStatusDtos dto,LiveStatusStore liveStatusStore) =>
{
    liveStatusStore.UpdateLiveStatus(dto.DoorOpen, dto.SoundValue);
    return Results.Ok();
}); 

// get live status
app.MapGet("/live-status", (LiveStatusStore liveStatusStore) =>
{
    var LiveStatus = liveStatusStore.GetLiveStatusDtos();
    return Results.Ok(LiveStatus);      
});


app.Run("http://0.0.0.0:5238");


