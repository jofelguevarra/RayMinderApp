using Microsoft.AspNetCore.Mvc;
using RayMinder.Api.Common;
using RayMinder.Api.Model;

// For more information on enabling Web API for empty projects, visit https://go.microsoft.com/fwlink/?LinkID=397860

namespace RayMinder.Api.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class UserController : ControllerBase
    {
        // In-memory user store for demo purposes
        private static readonly List<User> Users = new();

        [HttpPost("register")]
        public IActionResult Register([FromBody] User user)
        {
            //TODO: Implement proper validation and error handling
            //Create proper user registration mechanism with email verification for production use
            // Connect to a database to store user credentials securely (Sql, MongoDb)

            if (Users.Any(u => u.Username == user.Username))
                return BadRequest("Username already exists.");

            var hashedPassword = PasswordHelper.HashPassword(user.PasswordHash);
            Users.Add(new User { Username = user.Username, PasswordHash = hashedPassword });
            return Ok("User registered successfully.");
        }

        [HttpPost("login")]
        public IActionResult Login([FromBody] User user)
        {
            //TODO: Create proper authentication mechanism with tokens for production use
            // This is a simplified example for demonstration purposes only
            // In production, consider using ASP.NET Core Identity or JWT tokens
            // Connect to a database to validate user credentials securely (Sql, MongoDb)

            var hashedPassword = PasswordHelper.HashPassword(user.PasswordHash);
            var existingUser = Users.FirstOrDefault(u => u.Username == user.Username && u.PasswordHash == hashedPassword);

            if (existingUser == null)
                return Unauthorized("Invalid username or password.");

            return Ok("Login successful.");
        }
    }
}
