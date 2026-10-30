# Bomberman World — Short fuse!


**Student:** Ervin Behluli


## How to play

- **Controls:** Movement WASD or arrow keys, place bombs Space, detonate bombs E, restart R or Enter after winning or losing, quit Q or Esc.
- **Objective and rules:** To win you need to defeat all three enemies using bombs. You lose if you touch an enemy or get caught in an explosion. You can place up to three bombs at once, and explosions can trigger other bombs. Losing takes priority if you defeat the last enemy and die at the same time.
- **Configurations / demo setup:** Nothing additional needed.

## Features and validation

| Feature | Check, observed result, and expected behaviour |
| --- | --- |
| 1. Player damage and death | I checked damage from both enemies and explosions. Being close to an enemy without touching it caused no damage, while direct contact resulted in death - works as expected. |
| 2. Round states and objective | I checked that defeating all enemies results in a win and dying results in a loss. If the player dies while defeating the final enemy, losing takes priority. Matches expected behaviour. |
| 3. Safe restart | I checked restarting after both winning and losing. All bombs and explosions were cleared, enemies returned, and the player could play normally again. Old entity handles were also invalidated. |
| 4. Solid bombs with owner escape | Bombs allow the player to move away after placing them but prevent moving back through them. Placing another bomb on the same tile is rejected, and the tile becomes accessible again after the explosion. Works as expected. |
| 5. Blast-range pickup | The default blast range is three and increases by one when collecting a pickup, up to a maximum of six. I checked that collecting a pickup only affects newly placed bombs and that the same pickup cannot be collected twice. |
| 6. Multiple bombs per owner | I checked placing three bombs and attempting to place a fourth. The fourth bomb was rejected, but after the previous bombs exploded placing new bombs worked again. |
| 7. Chain reactions | I checked placing two bombs close enough for one explosion to reach the other. The second bomb exploded immediately when hit, without waiting for its timer, and did not explode a second time.|
| 13. Remote-detonated bombs | I checked detonating bombs manually using E. The bombs exploded immediately, and pressing E when no bombs were present caused no problems. New bombs could still be placed afterward. |
**Known issues:** None known after testing


## Sources and assistance

This project builds on the shared Kotte foundations. I used AI to assist in identifying and understanding the causes of bugs encountered during development.