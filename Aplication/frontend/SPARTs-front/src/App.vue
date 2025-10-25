<script setup>
import { Button, Column, InputText } from 'primevue';
import { ref } from 'vue';
const connected = ref (false);
const dialogSearch = ref (false);

const filters = ref({
  global: { value: null, matchMode: "contains" },
  name: { value: null, matchMode: "contains" },
});

// Exemplo de dados
const products = ref([
  { name: "Laptop", image: "bamboo-watch.jpg", quantity: 5 },
  { name: "Keyboard", image: "black-watch.jpg", quantity: 8 },
  { name: "Mouse", image: "blue-t-shirt.jpg", quantity: 3 },
]);

</script>

<template>
  <div>
    <div class="grid justify-content-center">
        <Button class="col-8" label="Connect to SPARTs" v-model:disabled="connected" />
    </div>
    <div class="grid justify-content-center mt-3">
        <Button class="col-8" label="Search item" @click="dialogSearch = true"/>
    </div>


    <Dialog v-model:visible="dialogSearch" modal header="Search Item" :style="{ width: '50rem' }">
      <div>
        <DataTable :value="products" :filters="filters" filterDisplay="menu" tableStyle="min-width: 50rem">
          <template #header>
          <div class="flex flex-wrap items-center justify-between gap-2">
            <span class="text-xl font-bold">Items</span>
            <!-- Campo de pesquisa -->
            <span class="p-input-icon-left">
              <i class="pi pi-search" />
              <InputText placeholder="Search by name" v-model="filters.name.value"/>
            </span>
          </div>
          </template>
          <Column header="Action">
            <Button label="Fetch" icon="pi pi-search" />
          </Column>  
          <Column field="name" header="Name"></Column>
          <Column header="Image">
            <template #body="slotProps">
              <img :src="`https://primefaces.org/cdn/primevue/images/product/${slotProps.data.image}`" :alt="slotProps.data.image" class="w-24 rounded" />
            </template>
          </Column>
          <Column field="quantity" header="Quantity"></Column>
        </DataTable>
      </div>
    </Dialog>
  </div>
</template>

<style scoped></style>
