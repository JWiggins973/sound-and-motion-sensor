using Microsoft.EntityFrameworkCore;

public class AlertDbContext : DbContext
{
    public DbSet<AlertEvent> Alerts { get; set; }

    protected override void OnConfiguring(DbContextOptionsBuilder options) => options.UseSqlite("Data Source=alerts.db");
}
