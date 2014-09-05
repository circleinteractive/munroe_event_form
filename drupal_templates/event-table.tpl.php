<table class="table table-bordered" style="margin-bottom:35px">
  <thead>
    <tr>
      <th>Event Title</th>
      <th>Date</th>
      <th>Time</th>
    </tr>
  </thead>
  <?php foreach ($events as $event): ?>
    <tr>
      <td><?php print $event['title']; ?></td>
      <td><?php print date('F jS, Y', strtotime($event['start_date'])); ?></td>
      <td><?php print date('g:i A', strtotime($event['start_date'])); ?></td>
    </tr>
  <?php endforeach; ?>
</table>