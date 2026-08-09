using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace AlertBackend.Migrations
{
    /// <inheritdoc />
    public partial class AddRawPeakToPeak : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.AddColumn<float>(
                name: "RawPeak",
                table: "Alerts",
                type: "REAL",
                nullable: false,
                defaultValue: 0f);
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropColumn(
                name: "RawPeak",
                table: "Alerts");
        }
    }
}
